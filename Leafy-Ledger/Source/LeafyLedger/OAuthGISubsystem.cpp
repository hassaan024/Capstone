// Fill out your copyright notice in the Description page of Project Settings.

#include "OAuthGISubsystem.h"
#include "Async/Async.h"
#include "SocketSubsystem.h"
#include "Networking.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"

static FString UrlEncode(const FString& In)
{
    return FGenericPlatformHttp::UrlEncode(In);
}

void UOAuthGISubsystem::BeginLoginViaBackendPush()
{
    // Generate sid
    ExpectedSid = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
    StartTimeSeconds = FPlatformTime::Seconds();

    if (!StartListener())
    {
        //UE_LOG(LogTemp, Warning, TEXT("Failed to start local listener"));

        OnOAuthLoginPushed.Broadcast(false, TEXT("Failed to start local listener"));
        return;
    }

    const int32 Port = BoundEndpoint.Port;
    const FString BaseGoogleAuthURL = TEXT("https://accounts.google.com/o/oauth2/v2/auth");

    const FString ClientID = TEXT("1083171967667-uepovjdmlhq1ah0dvjdkhefrteh4ujhj.apps.googleusercontent.com");
    const FString RedirectURI = TEXT("http://localhost:4000/backend/auth/google/unreal");

    const FString GoogleAuthURL = FString::Printf(
        TEXT("https://accounts.google.com/o/oauth2/v2/auth")
        TEXT("?client_id=%s")
        TEXT("&response_type=code")
        TEXT("&redirect_uri=%s")
        TEXT("&scope=openid%%20email%%20profile")
        TEXT("&state=%s"),
        *ClientID,
        *RedirectURI,
        *ExpectedSid
    );

    ///start?sid=%s&return_port=%d"), *UrlEncode(ExpectedSid), Port
    //UE_LOG(LogTemp, Warning, TEXT("OAuth UE Listener bound: %s"), *BoundEndpoint.ToString());
    //UE_LOG(LogTemp, Warning, TEXT("Opening browser to backend start URL:\n%s"), *BackendStartUrl);

    FPlatformProcess::LaunchURL(*GoogleAuthURL, nullptr, nullptr);
}

void UOAuthGISubsystem::CancelLoginListener()
{
    StopListener();
    ExpectedSid.Empty();
}

bool UOAuthGISubsystem::StartListener()
{
    //UE_LOG(LogTemp, Log, TEXT("Starting TCP listener..."));

    if (bListening.Load())
    {
        //UE_LOG(LogTemp, Warning, TEXT("Listener already running"));
        return true;
    }

    ISocketSubsystem* Sockets = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!Sockets)
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to get socket subsystem"));
        return false;
    }

    TSharedRef<FInternetAddr> Addr = Sockets->CreateInternetAddr();
    constexpr int32 Port = 50000;
    Addr->SetAnyAddress();
    Addr->SetPort(Port);

    ListenSocket = Sockets->CreateSocket(NAME_Stream, TEXT("UE_OAuth_Push_Listener"), false);
    if (!ListenSocket)
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to create listen socket"));
        return false;
    }

    ListenSocket->SetReuseAddr(true);

    if (!ListenSocket->Bind(*Addr))
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to bind listen socket on port %d"), Port);
        StopListener();
        return false;
    }
    //UE_LOG(LogTemp, Log, TEXT("Successfully bound to port %d"), Port);

    if (!ListenSocket->Listen(8))
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to start listening on port %d"), Port);
        StopListener();
        return false;
    }
    //UE_LOG(LogTemp, Log, TEXT("Socket is now listening..."));

    // Read back bound endpoint (port chosen by OS)
    TSharedRef<FInternetAddr> BoundAddr = Sockets->CreateInternetAddr();
    ListenSocket->GetAddress(*BoundAddr);
    BoundEndpoint = FIPv4Endpoint(BoundAddr);

    bListening.Store(true);
    //UE_LOG(LogTemp, Log, TEXT("TCP Listener fully started. Bound endpoint: %s"), *BoundEndpoint.ToString());

    // Start background accept loop
    Async(EAsyncExecution::Thread, [this]()
        {
            //UE_LOG(LogTemp, Log, TEXT("Accept loop thread started"));
            AcceptLoop();
        });

    return true;
}

void UOAuthGISubsystem::StopListener()
{
    //UE_LOG(LogTemp, Warning, TEXT("StopListener() called"));

    bListening.Store(false);

    if (ListenSocket)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Closing listener socket"));
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
    }
    else
    {
        //UE_LOG(LogTemp, Warning, TEXT("StopListener(): ListenSocket was already null"));
    }
}

void UOAuthGISubsystem::AcceptLoop()
{
    //UE_LOG(LogTemp, Warning, TEXT("AcceptLoop started"));

    ISocketSubsystem* Sockets = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    while (bListening.Load())
    {
        // Timeout check
        const double Now = FPlatformTime::Seconds();
        if (TimeoutSeconds > 0 && (Now - StartTimeSeconds) > TimeoutSeconds)
        {
            //UE_LOG(LogTemp, Warning, TEXT("AcceptLoop timeout reached"));
            StopListener();

            AsyncTask(ENamedThreads::GameThread, [this]()
                {
                    OnOAuthLoginPushed.Broadcast(false, TEXT("Login timed out"));
                });
            return;
        }

        bool bPending = false;

        if (!ListenSocket)
        {
            //UE_LOG(LogTemp, Error, TEXT("AcceptLoop: ListenSocket is null"));
            return;
        }

        if (!ListenSocket->HasPendingConnection(bPending))
        {
            //UE_LOG(LogTemp, Error, TEXT("HasPendingConnection failed"));
            FPlatformProcess::Sleep(0.05f);
            continue;
        }

        if (!bPending)
        {
            // No incoming connection yet
            FPlatformProcess::Sleep(0.05f);
            continue;
        }

        //UE_LOG(LogTemp, Warning, TEXT("Pending connection detected"));

        TSharedRef<FInternetAddr> ClientAddr = Sockets->CreateInternetAddr();
        FSocket* Client = ListenSocket->Accept(*ClientAddr, TEXT("UE_OAuth_Push_Client"));

        if (!Client)
        {
            //UE_LOG(LogTemp, Error, TEXT("Accept() returned null socket"));
            FPlatformProcess::Sleep(0.05f);
            continue;
        }

        //UE_LOG(LogTemp, Warning, TEXT("Accepted connection from backend"));

        // ---- READ REQUEST ----
        TArray<uint8> Bytes;
        if (!ReadAllAvailable(Client, Bytes, 2.0))
        {
            //UE_LOG(LogTemp, Error, TEXT("Failed to read request bytes"));

            const FString Resp = MakeHttpResponse(400, TEXT("Bad Request"));
            FTCHARToUTF8 Utf8(*Resp);
            int32 Sent = 0;
            Client->Send((uint8*)Utf8.Get(), Utf8.Length(), Sent);

            Client->Close();
            Sockets->DestroySocket(Client);
            continue;
        }

        //UE_LOG(LogTemp, Warning, TEXT("Received %d bytes from backend"), Bytes.Num());

        FString ReqStr = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData())));
        ReqStr.ReplaceInline(TEXT("\0"), TEXT(""));

        FString Headers, Body;
        if (!SplitHeadersBody(ReqStr, Headers, Body))
        {
            //UE_LOG(LogTemp, Error, TEXT("Failed to split headers/body"));

            const FString Resp = MakeHttpResponse(400, TEXT("Bad Request"));
            FTCHARToUTF8 Utf8(*Resp);
            int32 Sent = 0;
            Client->Send((uint8*)Utf8.Get(), Utf8.Length(), Sent);

            Client->Close();
            Sockets->DestroySocket(Client);
            continue;
        }

        FString Method, Path;
        if (!ParseRequestLine(Headers, Method, Path))
        {
            //UE_LOG(LogTemp, Error, TEXT("Failed to parse request line"));

            const FString Resp = MakeHttpResponse(400, TEXT("Bad Request"));
            FTCHARToUTF8 Utf8(*Resp);
            int32 Sent = 0;
            Client->Send((uint8*)Utf8.Get(), Utf8.Length(), Sent);

            Client->Close();
            Sockets->DestroySocket(Client);
            continue;
        }

        //UE_LOG(LogTemp, Warning, TEXT("OAuth push request: method=%s path=%s bodyLen=%d"), *Method, *Path, Body.Len());

        // Only accept POST /oauth/complete
        if (!Method.Equals(TEXT("POST"), ESearchCase::IgnoreCase) ||
            !Path.Equals(TEXT("/oauth/complete"), ESearchCase::IgnoreCase))
        {
            //UE_LOG(LogTemp, Error, TEXT("Unexpected path or method"));

            const FString Resp = MakeHttpResponse(404, TEXT("Not Found"));
            FTCHARToUTF8 Utf8(*Resp);
            int32 Sent = 0;
            Client->Send((uint8*)Utf8.Get(), Utf8.Length(), Sent);

            Client->Close();
            Sockets->DestroySocket(Client);
            continue;
        }

        // ---- JSON PARSE ----
        TSharedPtr<FJsonObject> Json;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);

        if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
        {
            //UE_LOG(LogTemp, Error, TEXT("Invalid JSON body"));

            const FString Resp = MakeHttpResponse(400, TEXT("Invalid JSON"));
            FTCHARToUTF8 Utf8(*Resp);
            int32 Sent = 0;
            Client->Send((uint8*)Utf8.Get(), Utf8.Length(), Sent);

            Client->Close();
            Sockets->DestroySocket(Client);
            continue;
        }

        //UE_LOG(LogTemp, Warning, TEXT("JSON parsed successfully"));

        const FString Sid = Json->GetStringField(TEXT("sid"));
        FString SessionTokenLocal;
        if (Json->HasField(TEXT("session_token")))
        {
            SessionTokenLocal = Json->GetStringField(TEXT("session_token"));
        }
        else
        {
            SessionTokenLocal = TEXT(""); // fallback and log warning
            //UE_LOG(LogTemp, Warning, TEXT("No session_token in JSON"));
        }

        //UE_LOG(LogTemp, Warning, TEXT("Received sid=%s tokenLen=%d"), *Sid, SessionTokenLocal.Len());

        if (!Sid.Equals(ExpectedSid, ESearchCase::CaseSensitive))
        {
            //UE_LOG(LogTemp, Error, TEXT("SID mismatch: expected=%s got=%s"), *ExpectedSid, *Sid);

            const FString Resp = MakeHttpResponse(403, TEXT("sid mismatch"));
            FTCHARToUTF8 Utf8(*Resp);
            int32 Sent = 0;
            Client->Send((uint8*)Utf8.Get(), Utf8.Length(), Sent);

            Client->Close();
            Sockets->DestroySocket(Client);

            // Fail this login attempt (recommended)
            StopListener();
            AsyncTask(ENamedThreads::GameThread, [this]()
                {
                    bLoggedIn = false;
                    SessionToken.Empty();

                    const FString Err = TEXT("SID mismatch");
                    OnLoginFailed.Broadcast(Err);
                    OnOAuthLoginPushed.Broadcast(false, Err);
                });

            return;
        }

        // Success response
        {
            const FString Resp = MakeHttpResponse(200, TEXT("OK"));
            FTCHARToUTF8 Utf8(*Resp);
            int32 Sent = 0;
            Client->Send((uint8*)Utf8.Get(), Utf8.Length(), Sent);
        }

        Client->Close();
        Sockets->DestroySocket(Client);

        //UE_LOG(LogTemp, Warning, TEXT("OAuth push accepted successfully"));

        StopListener();

        // Update state + broadcast on GAME THREAD.
        // Capture SessionTokenLocal by value (safe).
        AsyncTask(ENamedThreads::GameThread, [this, SessionTokenLocal]()
            {
                bLoggedIn = true;
                SessionToken = SessionTokenLocal;

                //UE_LOG(LogTemp, Warning, TEXT("Auth: broadcasting OnLoginSucceeded"));
                OnLoginSucceeded.Broadcast();

                // Optional debug event
                OnOAuthLoginPushed.Broadcast(true, SessionTokenLocal);
            });

        return;
    }

    //UE_LOG(LogTemp, Warning, TEXT("AcceptLoop exited"));
}

bool UOAuthGISubsystem::ReadAllAvailable(FSocket* Socket, TArray<uint8>& OutBytes, double MaxSeconds)
{
    const double Start = FPlatformTime::Seconds();
    TArray<uint8> Buffer;
    Buffer.SetNumUninitialized(4096);

    // Read until we have headers and the full Content-Length body (or time out)
    int32 TotalRead = 0;
    FString Accum;

    while ((FPlatformTime::Seconds() - Start) < MaxSeconds)
    {
        uint32 Pending = 0;
        if (!Socket->HasPendingData(Pending))
        {
            FPlatformProcess::Sleep(0.01f);
            continue;
        }

        int32 Read = 0;
        const int32 ToRead = FMath::Min<int32>((int32)Pending, Buffer.Num());
        if (!Socket->Recv(Buffer.GetData(), ToRead, Read))
            break;

        if (Read > 0)
        {
            OutBytes.Append(Buffer.GetData(), Read);
            TotalRead += Read;

            FString Chunk = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Buffer.GetData()))).Left(Read);
            Accum += Chunk;

            // Log everything received
            //UE_LOG(LogTemp, Warning, TEXT("Read %d bytes from socket: %s"), Read, *Chunk);

            FString Headers, Body;
            if (SplitHeadersBody(Accum, Headers, Body))
            {
                const int32 CL = FindContentLength(Headers);
                if (CL >= 0 && Body.Len() >= CL)
                {
                    // We have the complete JSON body
                    UpdateSessionFromJson(Body);
                    return true;
                }
            }
        }
    }

    // Best effort: if we got anything, return true and let the parser handle it
    return OutBytes.Num() > 0;
}

bool UOAuthGISubsystem::SplitHeadersBody(const FString& Request, FString& OutHeaders, FString& OutBody)
{
    int32 Sep = Request.Find(TEXT("\r\n\r\n"));
    if (Sep == INDEX_NONE)
        Sep = Request.Find(TEXT("\n\n"));
    if (Sep == INDEX_NONE)
        return false;

    OutHeaders = Request.Left(Sep);
    OutBody = Request.Mid(Sep + 4); // assumes \r\n\r\n; okay for our backend
    return true;
}

bool UOAuthGISubsystem::ParseRequestLine(const FString& Headers, FString& OutMethod, FString& OutPath)
{
    // First line: METHOD PATH HTTP/1.1
    TArray<FString> Lines;
    Headers.ParseIntoArrayLines(Lines, true);
    if (Lines.Num() == 0)
        return false;

    TArray<FString> Parts;
    Lines[0].ParseIntoArray(Parts, TEXT(" "), true);
    if (Parts.Num() < 2)
        return false;

    OutMethod = Parts[0];
    OutPath = Parts[1];
    return true;
}

int32 UOAuthGISubsystem::FindContentLength(const FString& Headers)
{
    // Very simple parse
    int32 Pos = Headers.Find(TEXT("Content-Length:"), ESearchCase::IgnoreCase);
    if (Pos == INDEX_NONE)
        return -1;

    int32 LineEnd = Headers.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Pos);
    FString Line = (LineEnd == INDEX_NONE) ? Headers.Mid(Pos) : Headers.Mid(Pos, LineEnd - Pos);

    Line = Line.Replace(TEXT("\r"), TEXT(""));
    Line = Line.Replace(TEXT("Content-Length:"), TEXT(""), ESearchCase::IgnoreCase).TrimStartAndEnd();

    return FCString::Atoi(*Line);
}

FString UOAuthGISubsystem::MakeHttpResponse(int32 Code, const FString& Body)
{
    FString Status = TEXT("OK");
    if (Code == 400) Status = TEXT("Bad Request");
    if (Code == 403) Status = TEXT("Forbidden");
    if (Code == 404) Status = TEXT("Not Found");
    if (Code == 500) Status = TEXT("Internal Server Error");

    const FString Payload = Body;
    return FString::Printf(
        TEXT("HTTP/1.1 %d %s\r\nContent-Type: text/plain\r\nConnection: close\r\nContent-Length: %d\r\n\r\n%s"),
        Code, *Status, Payload.Len(), *Payload
    );
}

void UOAuthGISubsystem::UpdateSessionFromJson(const FString& JsonBody)
{
    TSharedPtr<FJsonObject> Obj;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);

    if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateSessionFromJson: invalid JSON: %s"), *JsonBody);
        return;
    }

    FString id, googleDisplayName;
    if (Obj->TryGetStringField(TEXT("id"), id))
    {
        Session.id = id;
        //UE_LOG(LogTemp, Log, TEXT("Session.id set to: %s"), *Session.id);
    }
    if (Obj->TryGetStringField(TEXT("googleDisplayName"), googleDisplayName)) 
    {
        Session.googleDisplayName = googleDisplayName;
        //UE_LOG(LogTemp, Log, TEXT("Session.googleDisplayName set to: %s"), *Session.googleDisplayName);
    }
}
