// ofs_client.cpp
#include <iostream>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

static const int SERVER_PORT = 9090;
static const char* SERVER_IP = "127.0.0.1";

int start_client() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv.sin_addr);

    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    cout << "✅ Connected to OFS server at "
         << SERVER_IP << ":" << SERVER_PORT << "\n";
    cout << "Type commands like:\n";
    cout << "  LOGIN|admin|admin123\n";
    cout << "  FORMAT\n";
    cout << "  CREATE_FILE|/projects/example.txt|hello world\n";
    cout << "  DELETE_FILE|/projects/example.txt\n";
    cout << "  QUIT\n\n";

    string line;
    char buffer[4096];

    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;

        // Add newline so server can trim if it wants
        line += "\n";

        ssize_t sent = send(sock, line.c_str(), line.size(), 0);
        if (sent <= 0) {
            cout << "❌ Failed to send / server closed.\n";
            break;
        }

        ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            cout << "❌ Connection closed by server.\n";
            break;
        }
        buffer[n] = '\0';
        cout << "< " << buffer;
        if (line.rfind("QUIT", 0) == 0) break;
    }

    close(sock);
    return 0;
}
