// Fill out your copyright notice in the Description page of Project Settings.


#include "OAuthGISubsystem.h"

namespace
{
    FString MakeRandomState()
    {
        return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
    }

    FString UrlEncode(const FString& In)
    {
        return FGenericPlatformHttp::UrlEncode(In);
    }
}

void UOAuthGISubsystem::BeginGoogleLogin()
{
    if (!StartLoopbackListener())
    {
        OnGoogleLoginResult.Broadcast(false, TEXT("Failed to start loopback listener"));
        return;
    }

    const FString State = MakeRandomState();

    const int32 Port = LocalEndpoint.Port;
    const FString RedirectUri = FString::Printf(TEXT("http://127.0.0.1:%d/callback"), Port);

    const FString AuthUrl =
        TEXT("https://accounts.google.com/o/oauth2/v2/auth")
        TEXT("?client_id=") + UrlEncode(ClientId) +
        TEXT("&redirect_uri=") + UrlEncode(RedirectUri) +
        TEXT("&response_type=code") +
        TEXT("&scope=") + UrlEncode(Scope) +
        TEXT("&access_type=offline") +
        TEXT("&prompt=select_account") +
        TEXT("&state=") + UrlEncode(State);

    OpenSystemBrowser(AuthUrl);

    // Accept loop runs in background; it will call ExchangeCodeForTokens when it receives a code.
}

bool UOAuthGISubsystem::StartLoopbackListener()
{
    if (bListening.Load()) return true;

    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem) return false;

    // Bind to 127.0.0.1:0 (port 0 = choose a free port)
    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid = false;
    Addr->SetIp(TEXT("127.0.0.1"), bIsValid);
    Addr->SetPort(0);
    if (!bIsValid) return false;

    ListenSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("GoogleOAuthLoopback"), false);
    if (!ListenSocket) return false;

    ListenSocket->SetReuseAddr(true);

    if (!ListenSocket->Bind(*Addr))
    {
        StopLoopbackListener();
        return false;
    }

    if (!ListenSocket->Listen(1))
    {
        StopLoopbackListener();
        return false;
    }

    // Read back the chosen port
    TSharedRef<FInternetAddr> BoundAddr = SocketSubsystem->CreateInternetAddr();
    ListenSocket->GetAddress(*BoundAddr);

    LocalEndpoint = FIPv4Endpoint(BoundAddr);

    bListening.Store(true);

    // Background accept thread (simple)
    Async(EAsyncExecution::Thread, [this]()
        {
            RunAcceptLoop();
        });

    return true;
}

void UOAuthGISubsystem::StopLoopbackListener()
{
    bListening.Store(false);

    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
    }
}

void UOAuthGISubsystem::OpenSystemBrowser(const FString& AuthUrl)
{
    UE_LOG(LogTemp, Warning, TEXT("Google Auth URL:\n%s"), *AuthUrl);

    // Opens user’s default browser (Windows)
    FPlatformProcess::LaunchURL(*AuthUrl, nullptr, nullptr);
}

void UOAuthGISubsystem::RunAcceptLoop()
{
    // Accept a single callback then stop.
    while (bListening.Load())
    {
        bool bHasPending = false;
        if (!ListenSocket || !ListenSocket->HasPendingConnection(bHasPending))
        {
            FPlatformProcess::Sleep(0.01f);
            continue;
        }

        if (!bHasPending)
        {
            FPlatformProcess::Sleep(0.01f);
            continue;
        }

        TSharedRef<FInternetAddr> ClientAddr =
            ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

        FSocket* ClientSocket = ListenSocket->Accept(*ClientAddr, TEXT("GoogleOAuthClient"));
        if (!ClientSocket)
        {
            FPlatformProcess::Sleep(0.01f);
            continue;
        }

        // Read HTTP request (very small)
        TArray<uint8> Buffer;
        Buffer.SetNumUninitialized(8192);

        int32 BytesRead = 0;
        ClientSocket->Recv(Buffer.GetData(), Buffer.Num(), BytesRead);

        FString Request = BytesRead > 0
            ? FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Buffer.GetData()))).Left(BytesRead)
            : FString();

        FString Code;
        const bool bGotCode = ParseRequestForCode(Request, Code);

        // Respond to browser
        const FString Html =
            TEXT("<html><body><h3>Login complete.</h3>You may close this tab.</body></html>");

        const FString Response =
            TEXT("HTTP/1.1 200 OK\r\n")
            TEXT("Content-Type: text/html\r\n")
            TEXT("Connection: close\r\n")
            TEXT("Content-Length: ") + FString::FromInt(Html.Len()) + TEXT("\r\n\r\n") +
            Html;

        FTCHARToUTF8 Utf8(*Response);
        int32 BytesSent = 0;
        ClientSocket->Send((uint8*)Utf8.Get(), Utf8.Length(), BytesSent);

        ClientSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);

        // Stop listening now
        StopLoopbackListener();

        if (bGotCode)
        {
            const FString RedirectUri =
                FString::Printf(TEXT("http://127.0.0.1:%d/callback"), LocalEndpoint.Port);

            // Back to game thread for HTTP calls + delegates
            AsyncTask(ENamedThreads::GameThread, [this, Code, RedirectUri]()
                {
                    ExchangeCodeForTokens(Code, RedirectUri);
                });
        }
        else
        {
            AsyncTask(ENamedThreads::GameThread, [this]()
                {
                    OnGoogleLoginResult.Broadcast(false, TEXT("OAuth callback did not contain a code"));
                });
        }

        return;
    }
}

bool UOAuthGISubsystem::ParseRequestForCode(const FString& HttpRequest, FString& OutCode)
{
    // Expect: GET /callback?code=...&scope=... HTTP/1.1
    int32 GetPos = HttpRequest.Find(TEXT("GET "));
    if (GetPos == INDEX_NONE) return false;

    int32 PathStart = GetPos + 4;
    int32 PathEnd = HttpRequest.Find(TEXT(" HTTP/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, PathStart);
    if (PathEnd == INDEX_NONE) return false;

    const FString Path = HttpRequest.Mid(PathStart, PathEnd - PathStart);

    FString Query;
    FString Route;
    if (!Path.Split(TEXT("?"), &Route, &Query)) return false;

    // Parse query params
    TArray<FString> Parts;
    Query.ParseIntoArray(Parts, TEXT("&"), true);

    for (const FString& P : Parts)
    {
        FString K, V;
        if (P.Split(TEXT("="), &K, &V))
        {
            if (K == TEXT("code"))
            {
                OutCode = FGenericPlatformHttp::UrlDecode(V);
                return !OutCode.IsEmpty();
            }
        }
    }
    return false;
}

void UOAuthGISubsystem::ExchangeCodeForTokens(const FString& Code, const FString& RedirectUri)
{
    // POST https://oauth2.googleapis.com/token (x-www-form-urlencoded)
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(TEXT("https://oauth2.googleapis.com/token"));
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

    const FString Body =
        TEXT("code=") + UrlEncode(Code) +
        TEXT("&client_id=") + UrlEncode(ClientId) +
        TEXT("&client_secret=") + UrlEncode(ClientSecret) +
        TEXT("&redirect_uri=") + UrlEncode(RedirectUri) +
        TEXT("&grant_type=authorization_code");

    Req->SetContentAsString(Body);

    Req->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (!bSucceeded || !Response.IsValid())
            {
                OnGoogleLoginResult.Broadcast(false, TEXT("Token request failed (no response)"));
                return;
            }

            if (Response->GetResponseCode() != 200)
            {
                OnGoogleLoginResult.Broadcast(false,
                    FString::Printf(TEXT("Token request failed HTTP %d: %s"),
                        Response->GetResponseCode(), *Response->GetContentAsString()));
                return;
            }

            // Parse JSON
            TSharedPtr<FJsonObject> JsonObj;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
            {
                OnGoogleLoginResult.Broadcast(false, TEXT("Failed to parse token JSON"));
                return;
            }

            const FString IdToken = JsonObj->GetStringField(TEXT("id_token"));
            if (IdToken.IsEmpty())
            {
                OnGoogleLoginResult.Broadcast(false, TEXT("No id_token returned"));
                return;
            }

            // Success: return id_token (JWT). You can decode/verify, or send to your backend for verification.
            OnGoogleLoginResult.Broadcast(true, IdToken);

            GetGameInstance()->GetSubsystem<UOAuthGISubsystem>()->MarkLoginSucceeded();

        });

    Req->ProcessRequest();
}
