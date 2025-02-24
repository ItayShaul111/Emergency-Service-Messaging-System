// package bgu.spl.net.impl.echo;

// import java.io.BufferedReader;
// import java.io.BufferedWriter;
// import java.io.IOException;
// import java.io.InputStreamReader;
// import java.io.OutputStreamWriter;
// import java.net.Socket;

// public class EchoClient {

//     public static void main(String[] args) throws IOException {

//         if (args.length == 0) {
//             args = new String[]{"localhost", "hello"};
//         }

//         if (args.length < 2) {
//             System.out.println("you must supply two arguments: host, message");
//             System.exit(1);
//         }

//         //BufferedReader and BufferedWriter automatically using UTF-8 encoding
//         try (Socket sock = new Socket(args[0], 7777);
//                 BufferedReader in = new BufferedReader(new InputStreamReader(sock.getInputStream()));
//                 BufferedWriter out = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()))) {

//             System.out.println("sending message to server");
//             out.write(args[1]);
//             out.newLine();
//             out.flush();

//             System.out.println("awaiting response");
//             String line = in.readLine();
//             System.out.println("message from server: " + line);
//         }
//     }
// }
package bgu.spl.net.impl.echo;

import bgu.spl.net.impl.echo.LineMessageEncoderDecoder;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Scanner;

public class EchoClient {

    public static void main(String[] args) throws IOException {

        if (args.length == 0) {
            args = new String[]{"localhost"}; // Default host
        }

        if (args.length < 1) {
            System.out.println("You must supply at least one argument: host");
            System.exit(1);
        }

        try (Socket sock = new Socket(args[0], 7777); // Connect to the server on port 7777
             InputStream in = sock.getInputStream();
             OutputStream out = sock.getOutputStream();
             Scanner sc = new Scanner(System.in)) {

            LineMessageEncoderDecoder encdec = new LineMessageEncoderDecoder(); // Encoder-decoder instance

            String nextMessage;

            while (true) {
                // Check if the socket is closed
                if (sock.isClosed()) {
                    System.out.println("Connection closed by the server.");
                    break;
                }

                System.out.println("Enter a message to send to the server (use \\n for newlines, type 'bye' to quit):");
                nextMessage = sc.nextLine(); // Read user input

                // Replace literal '\n' with actual newlines
                nextMessage = nextMessage.replace("\\n", "\n");

                // Encode and send the message
                out.write(encdec.encode(nextMessage));
                out.flush();

                System.out.println("Awaiting response...");

                // Decode the server's response
                StringBuilder serverMessage = new StringBuilder();
                int read;

                while ((read = in.read()) != -1) {
                    String decoded = encdec.decodeNextByte((byte) read);
                    if (decoded != null) {
                        serverMessage.append(decoded);
                        break; // Stop when a full message is received
                    }
                }

                // Exit if the server has closed the connection
                // if (!messageReceived) {
                //     System.out.println("Connection closed by the server.");
                //     break;
                // }

                // Print the response from the server
                System.out.println("Message from server:");
                System.out.println(serverMessage.toString());

                // Exit if "bye" is entered
                if (serverMessage.toString().equals("bye")) {
                    System.out.println("Exiting client...");
                    break;
                }
            }
        } catch (IOException e) {
            System.out.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}