from tkinter import *
from tkinter import ttk
import socket
from threading import Thread

root = Tk()
root.title("Chat Room")

nameCheck = True

# Send function
def send(event=None):
    # Get message from text field
    sending_message = my_message.get()
    message_list.insert(END, "You: " + sending_message)

    # Clear the input from my_message since we already got it
    my_message.set("")
    # Send the message to the server over the given address
    sock.sendto(sending_message.encode(), serverAddr)
    # Closes socket and window if the user types "exit"
    if sending_message == "exit":
        try:
            sock.close()
        except EXCEPTION:
            pass

        message_frame.quit()

# Receive function
def receive():
    mess = "1"
    # Server responds with an empty string is socket is closed, therefore checking to ensure server is running
    while mess != "":
        # Try to retrieve the message from the socket and add it to the display
        try:
            incoming, addr = sock.recvfrom(10240)
            mess = incoming.decode()
            message_list.insert(END, mess)
        except OSError:
            break

    # Server shuts down on empty string so send the message, attempt to close socket if it is open, then close client
    if mess == "":
        message_list.insert(END, "Server has shutdown")
        try:
            sock.close()
        except EXCEPTION:
            pass

        message_frame.quit()

# Closing the client via the X sends an exit message in the textbox to close the socket
def on_closing():
    my_message.set("exit")
    send()

# Messages frame
message_frame = Frame(root)

# scroll element for browsing messages
scroll_bar = ttk.Scrollbar(message_frame)

message_list = Listbox(message_frame, height=25,
                       width=100, yscrollcommand=scroll_bar.set, bg='#d9d9d9')

# Packing scroll_bar, message_list and message_frame
scroll_bar.pack(side=RIGHT, fill=Y)
message_list.pack(side=LEFT, fill=BOTH)
message_list.pack()
message_frame.pack()

# User input variable
my_message = StringVar()
my_message.set("")

# Message Entry
# textvariable to retrieve data from my_message input
message_entry = Entry(root, textvariable=my_message)

# User have an option of pressing enter to send
message_entry.bind('<Return>', send)
message_entry.pack()

# If they want to use the Send button
send_button = Button(root, text="Send", command=send, width=5)
send_button.pack()

# Initialize the X to run the specified function
root.protocol("WM_DELETE_WINDOW", on_closing)

# Declare host ip and port number
host = "192.168.2.43"
port = 3000
serverAddr = (host, port)

# Initialize TCP socket and connect to address
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(serverAddr)

# Start the thread to check for incoming messages from server
receive_thread = Thread(target=receive)
receive_thread.start()

root.mainloop()
