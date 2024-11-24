-- Table to store client information
CREATE TABLE clients (
    client_id INTEGER PRIMARY KEY AUTOINCREMENT,        -- Unique ID for each client
    username VARCHAR(64) NOT NULL UNIQUE,               -- Username, must be unique
    password VARCHAR(255) NOT NULL,                     -- Password for authentication
    nickname VARCHAR(64)                                -- Optional nickname for the client
);

-- Table to store channel information
CREATE TABLE channels (
    channel_id INTEGER PRIMARY KEY AUTOINCREMENT,       -- Unique ID for each channel
    channel_name VARCHAR(64) NOT NULL UNIQUE,           -- Channel name, must be unique
    description TEXT,                                   -- Description of the channel
    key VARCHAR(64),                                    -- Optional key for restricted channels
    owner_id INTEGER NOT NULL,                          -- ID of the client who owns the channel
    FOREIGN KEY (owner_id) REFERENCES clients(client_id) ON DELETE CASCADE  -- If owner is deleted, delete the channel
);

-- Table to manage client memberships in channels
CREATE TABLE channel_memberships (
    channel_id INTEGER,                                 -- ID of the channel
    client_id INTEGER,                                  -- ID of the client
    role TEXT CHECK(role IN ('member', 'admin', 'moderator')) DEFAULT 'member',  -- Role of the client in the channel
    PRIMARY KEY (channel_id, client_id),                -- Composite primary key to prevent duplicate memberships
    FOREIGN KEY (channel_id) REFERENCES channels(channel_id) ON DELETE CASCADE,  -- Cascade delete if channel is removed
    FOREIGN KEY (client_id) REFERENCES clients(client_id) ON DELETE CASCADE      -- Cascade delete if client is removed
);

-- Table to store channel messages (messages within channels)
CREATE TABLE channel_messages (
    message_id INTEGER PRIMARY KEY AUTOINCREMENT,       -- Unique ID for each message
    sender_id INTEGER NOT NULL,                         -- ID of the client who sent the message
    channel_id INTEGER NOT NULL,                        -- ID of the channel where the message is posted
    message_text TEXT NOT NULL,                         -- Content of the message
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,        -- Timestamp of when the message was sent
    FOREIGN KEY (sender_id) REFERENCES clients(client_id) ON DELETE CASCADE,     -- Cascade delete if sender is removed
    FOREIGN KEY (channel_id) REFERENCES channels(channel_id) ON DELETE CASCADE   -- Cascade delete if channel is removed
);

-- Table to store private client-to-client messages
CREATE TABLE private_messages (
    message_id INTEGER PRIMARY KEY AUTOINCREMENT,       -- Unique ID for each message
    sender_id INTEGER NOT NULL,                         -- ID of the client who sent the message
    recipient_id INTEGER NOT NULL,                      -- ID of the client who receives the message
    message_text TEXT NOT NULL,                         -- Content of the message
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,        -- Timestamp of when the message was sent
    FOREIGN KEY (sender_id) REFERENCES clients(client_id) ON DELETE CASCADE,     -- Cascade delete if sender is removed
    FOREIGN KEY (recipient_id) REFERENCES clients(client_id) ON DELETE CASCADE   -- Cascade delete if recipient is removed
);

-- Table to store file information
CREATE TABLE files (
    file_id INTEGER PRIMARY KEY AUTOINCREMENT,          -- Unique ID for each file entry
    sender_id INTEGER NOT NULL,                          -- ID of the client who sends the file
    filename VARCHAR(255) NOT NULL,                      -- Name of the file
    recipient_id INTEGER,                                -- ID of the client receiving the file (nullable if sent to channel)
    channel_id INTEGER,                                  -- ID of the channel receiving the file (nullable if sent to client)
    uuid CHAR(36) NOT NULL,                              -- UUID of the file (unique identifier)
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,         -- Timestamp of when the file was sent
    FOREIGN KEY (sender_id) REFERENCES clients(client_id) ON DELETE CASCADE,  -- Cascade delete if sender is removed
    FOREIGN KEY (recipient_id) REFERENCES clients(client_id) ON DELETE CASCADE, -- Cascade delete if recipient is removed
    FOREIGN KEY (channel_id) REFERENCES channels(channel_id) ON DELETE CASCADE, -- Cascade delete if channel is removed
    CHECK (
        (recipient_id IS NOT NULL AND channel_id IS NULL) OR 
        (recipient_id IS NULL AND channel_id IS NOT NULL)
    )  -- Ensures file is either sent to a client or to a channel, not both
);

-- Create indexes after the table creation
CREATE INDEX idx_owner_id ON channels(owner_id);
CREATE INDEX idx_channel_id_memberships ON channel_memberships(channel_id);
CREATE INDEX idx_client_id_memberships ON channel_memberships(client_id);
CREATE INDEX idx_channel_id_messages ON channel_messages(channel_id);
CREATE INDEX idx_sender_id_private_messages ON private_messages(sender_id);
CREATE INDEX idx_recipient_id_private_messages ON private_messages(recipient_id);
CREATE INDEX idx_sender_id_files ON files(sender_id);
CREATE INDEX idx_recipient_id_files ON files(recipient_id);
CREATE INDEX idx_channel_id_files ON files(channel_id);

-- Admin User
INSERT INTO clients (username, password, nickname) VALUES ('sysadmin', 'BW3PD5!3tVcf&fE2', NULL);

-- Default channel
INSERT INTO channels (channel_name, description, key, owner_id)
VALUES (
    'default', 
    'Welcome to the default channel of server CS744. You can see the list of users and channels here.',
    NULL,
    (SELECT client_id FROM clients WHERE username = 'sysadmin')
);

-- Add Admin user to Default channel
INSERT INTO channel_memberships (channel_id, client_id, role)
VALUES (
    (SELECT channel_id FROM channels WHERE channel_name = 'default'),
    (SELECT client_id FROM clients WHERE username = 'sysadmin'),
    'admin'
);

-- All user chat channel
INSERT INTO channels (channel_name, description, key, owner_id)
VALUES (
    'chat', 
    'Welcome to the global channel of server CS744. Everyone can talk here',
    NULL,
    (SELECT client_id FROM clients WHERE username = 'sysadmin')
);

-- Add Admin user to Default channel
INSERT INTO channel_memberships (channel_id, client_id, role)
VALUES (
    (SELECT channel_id FROM channels WHERE channel_name = 'chat'),
    (SELECT client_id FROM clients WHERE username = 'sysadmin'),
    'admin'
);


-- Insert sample data for channel message
INSERT INTO channel_messages (sender_id, channel_id, message_text)
VALUES (
    (SELECT client_id FROM clients WHERE username = 'sysadmin'),
    (SELECT channel_id FROM channels WHERE channel_name = 'default'),
    'Welcome to the default channel!'
);

-- Insert sample data for private message
-- INSERT INTO private_messages (sender_id, recipient_id, message_text)
-- VALUES (
--     (SELECT client_id FROM clients WHERE username = 'sysadmin'),
--     (SELECT client_id FROM clients WHERE username = 'other_user'),
--     'Hello, how are you?'
-- );
