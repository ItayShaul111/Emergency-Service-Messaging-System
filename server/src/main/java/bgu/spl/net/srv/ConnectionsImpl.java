package bgu.spl.net.srv;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.impl.stomp.User;

public class ConnectionsImpl<T> implements Connections<T> {

    private final ConcurrentHashMap<Integer, ConnectionHandler<T>> activeConnections; // ConnectionID --> ConnectionHandler
    private final ConcurrentHashMap<String, ConcurrentHashMap<Integer,User>> channelSubscriptions; // Channel --> List of subs (Users)
    private final ConcurrentHashMap<String, User> allUsers; // Username --> User
    private AtomicInteger messageIDGen;

    private static class connectionsImplHolder {
        private static final ConnectionsImpl instance = new ConnectionsImpl<>();
    }

    public static ConnectionsImpl getInstance() {
        return connectionsImplHolder.instance;
    }

    // Constructor
    public ConnectionsImpl() {
        this.activeConnections = new ConcurrentHashMap<>();
        this.channelSubscriptions = new ConcurrentHashMap<>();
        this.allUsers = new ConcurrentHashMap<>();
        this.messageIDGen = new AtomicInteger(1);
    }

    @Override
    public boolean send(int connectionId, T msg) {
        synchronized (activeConnections) { 
            ConnectionHandler<T> handler = activeConnections.get(connectionId);
            if (handler != null) {
                handler.send(msg);
                return true;
            }
            return false;
        }
    }


    @Override
    public void send(String channel, T msg) {
        synchronized (channelSubscriptions) { 
            ConcurrentHashMap<Integer,User> subscribersMap = channelSubscriptions.get(channel);
            if (subscribersMap != null) {
                for (User user : subscribersMap.values()) {
                    send(user.getConnectionId(), msg);
                }
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        // Deleting from connectID-CH map
        activeConnections.remove(connectionId);
        // Delete from all subscribed channels
        for (Map<Integer,User> subscribersMap : channelSubscriptions.values()) {
            subscribersMap.remove(connectionId);
        }
    }

    public ConnectionHandler<T> getConnectionHandler(int id) {
        return activeConnections.get(id);
    }

    public void connect(int connectionId, ConnectionHandler<T> handler) {
        activeConnections.put(connectionId, handler);
    }

    public User getUser(String userName) {
        return allUsers.get(userName);
    }

    public void addUser(User user) {
        allUsers.put(user.getUserName(), user);
    }

    public void subscribe(String channel, int connectionID, User user) {
        synchronized (channelSubscriptions) {  
            if (!channelSubscriptions.containsKey(channel)) {
                channelSubscriptions.put(channel, new ConcurrentHashMap<Integer,User>());
            }
            channelSubscriptions.get(channel).put(connectionID, user);
        }
    }

    public void unsubscribe(String channel, int connectionID) {
        synchronized (channelSubscriptions) {  
            channelSubscriptions.get(channel).remove(connectionID);
        }
    }

    public int getMessageID() {
        return this.messageIDGen.incrementAndGet();
    }


    public ConcurrentHashMap<Integer, User> getSubsToChannel(String channel) {
        return channelSubscriptions.get(channel);
    }

}