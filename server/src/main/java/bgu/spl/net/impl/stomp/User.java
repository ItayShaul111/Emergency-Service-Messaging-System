package bgu.spl.net.impl.stomp;

import java.util.Map;
import java.util.HashMap;

public class User {
    private int connectionId;
    private String userName;
    private String password;
    private boolean isConnected;
    private Map<String, String> subscribeIdToChannels; 

    public User(String userName, String password) {
        this.connectionId = -1;
        this.userName = userName;
        this.password = password;        
        this.isConnected = false;
        this.subscribeIdToChannels = new HashMap<>();
    }

    // GETTERS
    public int getConnectionId() {
        return connectionId;
    }

    public String getUserName() {
        return userName;
    }

    public String getPassword() {
        return password;
    }

    public boolean isConnected() {
        return isConnected;
    }

    public void addSub(String channel, String subscriptionID) {
        this.subscribeIdToChannels.put(subscriptionID, channel);
    }

    public boolean isSubscribedWithId(String subscriptionID) {
        return subscribeIdToChannels.containsKey(subscriptionID);
    }

    public String removeSub(String subscriptionID){
        return this.subscribeIdToChannels.remove(subscriptionID);
    }

    public String getSubscriptionID (String channel) {
        for (Map.Entry<String, String> entry : subscribeIdToChannels.entrySet()) {
            if (entry.getValue().equals(channel)) {
                return entry.getKey();
            }
        }
        return null;
    }

    public boolean isSubscribedTo(String channel) {
        for (String subbedChannel : subscribeIdToChannels.values()) {
            if(subbedChannel.equals(channel)) {
                return true;
            }
        }
        return false;
    }

    // SETTERS
    public void setConnectionId(int connectionId) {
        this.connectionId = connectionId;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }

    public void disconnect() {
        setConnected(false);
        setConnectionId(-1);
        subscribeIdToChannels = new HashMap<>();
    }



}
