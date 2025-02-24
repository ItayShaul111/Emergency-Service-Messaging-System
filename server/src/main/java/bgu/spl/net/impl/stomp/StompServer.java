package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("Usage: java StompServer <port> <reactor/tpc>");
            return;
        }
        
        int port;
        try {
            port = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            System.out.println("Error: The port must be a valid integer.");
            return;
        }

        String serverType = args[1].toLowerCase();

        if (serverType.equals("tpc")) {
            Server.threadPerClient(
                    port, // port
                    StompMessagingProtocolImpl::new, // protocol factory
                    StompMessageEncoderDecoder::new // message encoder decoder factory
            ).serve();
        } else if (serverType.equals("reactor")) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port, // port
                    StompMessagingProtocolImpl::new, // protocol factory
                    StompMessageEncoderDecoder::new // message encoder decoder factory
            ).serve();
        } else {
            System.out.println("Error: Invalid server type. Use 'reactor' or 'tpc'.");
        }
    }
}
