import { UpdateUserDto } from 'user/dto/update-user.dto';
import { UNREAL_LISTENING_PORT } from './constants';
import { Socket } from 'net';

export async function sendGoogleAuthInfoToUnrealTCP(user_info: UpdateUserDto) {
  return new Promise((resolve, reject) => {
    // Map to safe DTO for Unreal
    const userDtoForUnreal = {
      email: user_info.email ?? '',
      displayName: user_info.displayName ?? '',
      passwordHash: user_info.passwordHash ?? '',
      googleId: user_info.googleId ?? '',
      verifiedEmail: user_info.verifiedEmail ? 'true' : 'false', // Unreal expects string
      googleDisplayName: user_info.googleDisplayName ?? '',
      givenName: user_info.givenName ?? '',
      familyName: user_info.familyName ?? '',
      picture: user_info.picture ?? '',
      sid: user_info.sid ?? '',
    };

    const client = new Socket();

    client.connect(UNREAL_LISTENING_PORT, 'host.docker.internal', () => {
      console.log('Connected to Unreal TCP server');

      const json = JSON.stringify(userDtoForUnreal);
      const httpRequest =
        `POST /oauth/complete HTTP/1.1\r\n` +
        `Host: localhost:${UNREAL_LISTENING_PORT}\r\n` +
        `Content-Type: application/json\r\n` +
        `Content-Length: ${Buffer.byteLength(json, 'utf8')}\r\n` +
        `\r\n` +
        json;

      client.write(httpRequest);
    });

    client.on('data', (data) => {
      try {
        // eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
        const message = JSON.parse(data.toString());
        console.log('Response from Unreal:', message);
        resolve(message);
      } catch (err) {
        console.log('Received from Unreal (raw):', data.toString());
        resolve(data.toString());
      }
      client.destroy();
    });

    client.on('error', (err) => {
      console.error('TCP connection error:', err);
      reject(err);
    });

    client.on('close', () => {
      console.log('TCP connection closed');
    });
  });
}