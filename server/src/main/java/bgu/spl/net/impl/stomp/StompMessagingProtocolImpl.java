package bgu.spl.net.impl.stomp;

import java.util.Map;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    private int connectionId;
    private ConnectionsImpl<String> connections;
    private User currentUser = null;


    @Override
    public void start(int connectionId, Connections<String> connections) {        
        this.connections = (ConnectionsImpl<String>) connections;
        this.connectionId = connectionId;
        System.out.println("protocol has started!");
    }

    @Override
    public void process(String message) {
        // Spliting msg
        System.out.println(message);
        String[] msgLines = message.split("\n");
        String command = msgLines[0];

        if (command.equals("CONNECT")) {
            handleConnect(msgLines);
            return;
        }
        if (currentUser == null) {
            sendErrorFrame("Cannot preform the action because user is not connected", "-1");
            return;
        }
        switch (command) {
            case "SUBSCRIBE":
                handleSubscribe(msgLines);
                break;
            case "UNSUBSCRIBE":
                handleUnsubscribe(msgLines);
                break;
            case "DISCONNECT":
                handleDisconnect(msgLines);
                break;    
            case "SEND":
                handleSend(msgLines);
                break;
            default:
                sendErrorFrame("Invalid command: " + command, "-1");
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    /************************************
     * CONNECT
     **************************************/
    private void handleConnect(String[] msgLines) {

        // Extract required headers
        String userName = splitHeaderValue(msgLines[3]);
        String password = splitHeaderValue(msgLines[4]);

        if (currentUser != null) {
            sendErrorFrame("The client is already logged in, log out before trying again.", "-1");
            return;
        }
        User user = connections.getUser(userName);
        // User never got created
        if (user != null) {
            if (!user.getPassword().equals(password)) {
                sendErrorFrame("Wrong password", "-1");
                return;
            }
            if (user.isConnected()) {
                sendErrorFrame("User already logged in", "-1");
                return;
            }
        }
        if (user == null) {
            user = new User(userName, password);
            connections.addUser(user);
        }
        currentUser = user;
        user.setConnected(true);
        user.setConnectionId(connectionId);

        connections.send(connectionId, "CONNECTED\nversion:1.2\n\n");
    }

    /************************************
     * DISCONNECT
     **************************************/
    private void handleDisconnect(String[] msgLines) {

        // Extract required header and send rececipt.
        String receiptID = splitHeaderValue(msgLines[1]);
        currentUser.disconnect();
        currentUser = null;

        connections.send(connectionId, "RECEIPT\nreceipt-id:" + receiptID + "\n\n");
        connections.disconnect(connectionId);
        
    }

    /************************************
     * SUBSCRIBE
     ***************************************/
    private void handleSubscribe(String[] msgLines) {
        String destination = splitHeaderValue(msgLines[1]);
        String subscriptionID = splitHeaderValue(msgLines[2]);
        String receiptID = splitHeaderValue(msgLines[3]);

        if(currentUser.isSubscribedTo(destination)) {
            sendErrorFrame(currentUser.getUserName() + " is already subscribed to channel: " + destination, receiptID);
            return;
        }

        currentUser.addSub(destination, subscriptionID);
        connections.subscribe(destination, connectionId, currentUser);
        
        connections.send(connectionId, "RECEIPT\nreceipt-id:" + receiptID + "\n\n");

    }

    /************************************
     * UNSUBSCRIBE
     ***************************************/
    private void handleUnsubscribe(String[] msgLines) {
        String subscriptionID = splitHeaderValue(msgLines[1]);
        String receiptID = splitHeaderValue(msgLines[2]);

        if (!currentUser.isSubscribedWithId(subscriptionID)) {
            sendErrorFrame("User isn't subscribed with this ID", receiptID);
            return;
        }

        String channel = currentUser.removeSub(subscriptionID);

        connections.unsubscribe(channel, connectionId);
        connections.send(connectionId, "RECEIPT\nreceipt-id:" + receiptID + "\n\n");

    }

    /************************************
     * SEND
     ***************************************/
    private void handleSend(String[] msgLines) {
        String destination = splitHeaderValue(msgLines[1]);
        
        if (currentUser.getSubscriptionID(destination) == null) {
            sendErrorFrame("User isn't subscribed to " + destination, "-1");
            return;
        }

        Map<Integer, User> destSubscribers = connections.getSubsToChannel(destination);

        String body = getBodyMessage(msgLines);

        for (User user : destSubscribers.values()) {
            String message = "MESSAGE\nsubscription: " + user.getSubscriptionID(destination) 
                    + "\nmessage-id: " + connections.getMessageID()
                    + "\ndestination: " + destination
                    + "\n\n"
                    + body
                    + "\n"
                    ;

            connections.send(user.getConnectionId(), message);
        }

    }
    
    public void sendErrorFrame(String msg, String recieptID) {
        if(recieptID == "-1") {
            connections.send(connectionId, "ERROR\nmessage: " + msg + "\n\n");
        } else {
            connections.send(connectionId, "ERROR\nreceipt-id: " + recieptID + "\nmessage: " + msg + "\n\n");
        }
        
        connections.disconnect(connectionId);
        if (currentUser != null) {
            currentUser.disconnect();
            currentUser = null;
        }

    }

    private String getBodyMessage(String[] msgLines) {
        String body = "";
        int index = 0;
        while (!msgLines[index].equals("")) {
            index++;
        }
        index++;
        while (index < msgLines.length) {
            body = body + msgLines[index] + "\n";
            index++;
        }

        return body;
    }

    public String splitHeaderValue(String header) {
        String[] headerParts = header.split(":");
        return headerParts[1];
    }
}