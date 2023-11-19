#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>


using namespace Sync;

class ConnectionThread : public Thread
{
private:
    Socket& socketReferenceC1;
    Socket& socketReferenceC2;
    int& serverThreadLife;
public:
    ConnectionThread(Socket& socketReferenceC1, Socket& socketReferenceC2,int& serverThreadLife) : socketReferenceC1(socketReferenceC1), socketReferenceC2(socketReferenceC2), serverThreadLife(serverThreadLife)
    {}

    ~ConnectionThread()
    {}

    virtual long ThreadMain() override
    {
    	//Send a prompt to the socket this thread is writing to informing them of the new user
	int c1Check = 1;
	std::string joinMes = "Another user has joined! Type 'exit' to leave.";
	ByteArray joinPrompt(joinMes);
	socketReferenceC1.Write(joinPrompt);
        
        //Loops as long as the server is not terminating and as long as the client does not close the socket
	while(serverThreadLife == 1 && c1Check > 0) {
		try{
			//Reads input from user 1
			ByteArray data("");
			c1Check = socketReferenceC1.Read(data);
			std::string userData = "";
			//Checks if user 1's socket is still open
			if(c1Check > 0) {
				userData = "Stranger: " + data.ToString();
			} else {
				userData = "Stranger has left the chat. Type 'exit' to leave";
			}
			//Sends the message to the other user
			ByteArray out(userData);
			socketReferenceC2.Write(out);
		} catch(std::string e) {}
        }
	return 0;
    }
};


// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    std::vector<Socket*> socks;
    int serverThreadLife = 1;
public:
    ServerThread(SocketServer& server)
    : server(server)
    {}

    ~ServerThread()
    {
    	//Loops through all sockets and closes them to ensure proper cleanup
    	for(int i = 0; i < socks.size(); i++) {
    		try{
    			socks[i]->Close();
    		} catch(...) {}
    	}
    	
    	serverThreadLife = 0; 
    }

    virtual long ThreadMain() override
    {
    	while(serverThreadLife == 1) {
    		try {
    			//Wait for first user connection
			Socket* newConnection = new Socket(server.Accept());
			socks.push_back(newConnection);
			Socket& socketReference = *newConnection;
			//Tell first user to wait until next user joins
			std::string waitMes = "Please wait for another user";
			ByteArray waitPrompt(waitMes);
			socketReference.Write(waitPrompt);		
			//Wait for second user once first user has joined
			Socket* newConnection2 = new Socket(server.Accept());
			socks.push_back(newConnection2);
			Socket& socketReference2 = *newConnection2;			
			//Initialize threads for both read/write combinations of the two users
			ConnectionThread* thread1 = new ConnectionThread(socketReference, socketReference2, serverThreadLife);
			ConnectionThread* thread2 = new ConnectionThread(socketReference2, socketReference, serverThreadLife);
    		} catch(std::string e) {
    			serverThreadLife = 0;
    			return 0;
    		}
	}
    	
	return 0;
    }
};

int main(void)
{
    std::cout << "I am a server." << std::endl;

    // Create our server
    SocketServer server(3000);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);

    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    // Shut down and clean up the server
    server.Shutdown();
}
