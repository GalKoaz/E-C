import socket
import os

host = '127.0.0.1'
port = 8080


def send_file(sock, filename):
    try:
        if os.path.isfile(filename):

            sock.send(filename.encode())

            with open(filename, "rb") as file:
                file_size = os.path.getsize(filename)
                file_size_bytes = file_size.to_bytes(8, byteorder="big")
                sock.send(file_size_bytes)

                while True:
                    chunk = file.read(1024)
                    if not chunk:
                        break
                    sock.send(chunk)
            print(f"File '{filename}' sent successfully.")
        else:
            print('File not found. Please enter a valid filename.')
        sock.send(b'EOF')
    except Exception as e:
        print(f"Error: {str(e)}")


def download_file(sock, filename):
    try:
        sock.send(filename.encode())

        with open(filename, "wb") as file:
            while True:
                chunk = sock.recv(1024)
                if not chunk:
                    break
                file.write(chunk)

        print(f"File '{filename}' received and saved successfully.")
    except Exception as e:
        print(f"Error: {str(e)}")


def show_files(sock):
    try:
        files_list = ""
        while True:
            chunk = sock.recv(1024).decode()
            if not chunk:
                break
            files_list += chunk

        print(f"Files available on the server:\n{files_list}")
    except Exception as e:
        print(f"Error: {str(e)}")


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    while True:
        operation = input("Type 'Upload' to upload a file, 'Download' to download a file,'Files' to see the files "
                          "already uploaded, or 'exit' to exit: ")
        if operation == 'Upload':
            filename = input('Enter the filename you want to send: ')
            sock.send(b'upload')
            send_file(sock, filename)
        elif operation == 'Download':
            filename = input('Enter the filename you want to download: ')
            sock.send(b'download')
            download_file(sock, filename)
        elif operation == 'Files':
            sock.send(b'list_files')
            show_files(sock)
        elif operation == 'exit':
            break
        else:
            print('Invalid operation. Please enter "Upload," "Download," or "exit".')

    sock.close()


if __name__ == '__main__':
    main()
