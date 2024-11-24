#include "database.hpp"

Database::Database(const std::string &dbName, int poolSize) : pool(dbName, poolSize) {}

Database::~Database() {}

bool Database::insertClient(const std::string &username, const std::string &password, int &client_id)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "INSERT INTO clients (username, password, nickname) VALUES (?, ?, NULL);";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the username and password to the statement
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Get the last inserted client_id
    client_id = static_cast<int>(sqlite3_last_insert_rowid(db));

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::getClientByUsername(const std::string &username, int &clientId, std::string &password, std::string &nickname)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "SELECT client_id, password, nickname FROM clients WHERE username = ?;";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the username to the statement
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result == SQLITE_ROW)
    {
        clientId = sqlite3_column_int(stmt, 0);
        password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));

        const char *nicknamePtr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        nickname = nicknamePtr ? nicknamePtr : "";
    }
    else if (result == SQLITE_DONE)
    {
        std::cerr << "No client found with the username: " << username << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }
    else
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::getClientByUsername(const std::string &username, int &clientId)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "SELECT client_id FROM clients WHERE username = ?;";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the username to the statement
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result == SQLITE_ROW)
    {
        clientId = sqlite3_column_int(stmt, 0);
    }
    else if (result == SQLITE_DONE)
    {
        std::cerr << "No client found with the username: " << username << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }
    else
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::insertChannel(const std::string &channelName, const std::string &description, int ownerId, const std::string &key)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "INSERT INTO channels (channel_name, description, key, owner_id) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the channel_name and description to the statement
    sqlite3_bind_text(stmt, 1, channelName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);

    // Bind the key, if provided, or NULL if not
    if (key.empty())
    {
        sqlite3_bind_null(stmt, 3);
    }
    else
    {
        sqlite3_bind_text(stmt, 3, key.c_str(), -1, SQLITE_STATIC);
    }

    // Bind the owner_id to the statement
    sqlite3_bind_int(stmt, 4, ownerId);

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::getAllChannelIds(std::vector<int> &channelIds)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "SELECT channel_id FROM channels;";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Execute the query and collect the channel_ids
    while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int channelId = sqlite3_column_int(stmt, 0);
        channelIds.push_back(channelId);
    }

    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::getChannel(int channel_id, std::string &name, std::string &description, std::string &key, int &creatorID, std::vector<int> &members, std::vector<int> &admins)
{
    members.clear();
    admins.clear();

    sqlite3 *db = pool.acquireConnection();
    const char *sql = R"(
        SELECT c.channel_name, c.description, c.key, c.owner_id, cm.client_id, cm.role
        FROM channels c
        LEFT JOIN channel_memberships cm ON c.channel_id = cm.channel_id
        WHERE c.channel_id = ?;
    )";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the channel_id to the statement
    sqlite3_bind_int(stmt, 1, channel_id);

    // Execute the statement
    while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (name.empty()) // Retrieve channel information only once
        {
            name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));

            const char *key_ptr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            key = key_ptr ? key_ptr : "";

            creatorID = sqlite3_column_int(stmt, 3);
        }

        int clientID = sqlite3_column_int(stmt, 4);
        const char *role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));

        members.push_back(clientID);

        if (role && std::string(role) == "admin")
        {
            admins.push_back(clientID);
        }
    }

    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::addMemberToChannel(int channel_id, int client_id, const std::string &role)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "INSERT INTO channel_memberships (channel_id, client_id, role) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, channel_id);                       // Bind channel_id
    sqlite3_bind_int(stmt, 2, client_id);                        // Bind client_id
    sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_STATIC); // Bind role

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::insertChannelMessage(int sender_id, int channel_id, const std::string &message_text)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "INSERT INTO channel_messages (sender_id, channel_id, message_text) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, sender_id);                                // Bind sender_id
    sqlite3_bind_int(stmt, 2, channel_id);                               // Bind channel_id
    sqlite3_bind_text(stmt, 3, message_text.c_str(), -1, SQLITE_STATIC); // Bind message_text

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::insertPrivateMessage(int sender_id, int recipient_id, const std::string &message_text)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "INSERT INTO private_messages (sender_id, recipient_id, message_text) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, sender_id);                                // Bind sender_id
    sqlite3_bind_int(stmt, 2, recipient_id);                             // Bind recipient_id
    sqlite3_bind_text(stmt, 3, message_text.c_str(), -1, SQLITE_STATIC); // Bind message_text

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::getChannelMessagesPaginated(int channel_id, int page, std::vector<std::tuple<std::string, std::string, std::string>> &messages)
{
    messages.clear();
    int offset = page * PAGE_SZ;

    sqlite3 *db = pool.acquireConnection();
    const char *sql = "SELECT c.username, cm.message_text, cm.sent_at "
                      "FROM channel_messages cm "
                      "JOIN clients c ON cm.sender_id = c.client_id "
                      "WHERE cm.channel_id = ? "
                      "ORDER BY cm.sent_at DESC "
                      "LIMIT ? OFFSET ?;";

    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, channel_id); // Bind channel_id
    sqlite3_bind_int(stmt, 2, PAGE_SZ);    // Bind PAGE_SZ
    sqlite3_bind_int(stmt, 3, offset);     // Bind offset (page * PAGE_SZ)

    // Execute the statement
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        const char *message_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *sent_at = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

        if (username && message_text && sent_at)
        {
            messages.push_back(std::make_tuple(username, message_text, sent_at));
        }
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return !messages.empty();
}

bool Database::getPrvMsgIds(int client_id, std::vector<int> &ids)
{
    ids.clear();

    sqlite3 *db = pool.acquireConnection();
    const char *sql = "SELECT DISTINCT sender_id "
                      "FROM private_messages "
                      "WHERE recipient_id = ? "
                      "UNION "
                      "SELECT DISTINCT recipient_id "
                      "FROM private_messages "
                      "WHERE sender_id = ?;";

    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, client_id); // Bind recipient_id = client_id
    sqlite3_bind_int(stmt, 2, client_id); // Bind sender_id = client_id

    // Execute the statement
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        ids.push_back(id);
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return !ids.empty();
}

bool Database::getPrivateMessagesPaginated(int id_a, int id_b, int page, std::vector<std::tuple<std::string, std::string, std::string>> &messages)
{
    messages.clear();
    int offset = page * PAGE_SZ;

    sqlite3 *db = pool.acquireConnection();
    const char *sql = "SELECT c.username, pm.message_text, pm.sent_at "
                      "FROM private_messages pm "
                      "JOIN clients c ON pm.sender_id = c.client_id "
                      "WHERE (pm.sender_id = ? AND pm.recipient_id = ?) "
                      "   OR (pm.sender_id = ? AND pm.recipient_id = ?) "
                      "ORDER BY pm.sent_at DESC "
                      "LIMIT ? OFFSET ?;";

    sqlite3_stmt *stmt;

    // Prepare the SQL statement
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false; // Return false on failure
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, id_a);    // Bind sender_id for the first condition
    sqlite3_bind_int(stmt, 2, id_b);    // Bind recipient_id for the first condition
    sqlite3_bind_int(stmt, 3, id_b);    // Bind sender_id for the second condition
    sqlite3_bind_int(stmt, 4, id_a);    // Bind recipient_id for the second condition
    sqlite3_bind_int(stmt, 5, PAGE_SZ); // Bind PAGE_SZ
    sqlite3_bind_int(stmt, 6, offset);  // Bind offset (page * PAGE_SZ)

    // Execute the statement
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        const char *message_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *sent_at = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

        if (username && message_text && sent_at)
        {
            messages.push_back(std::make_tuple(username, message_text, sent_at));
        }
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return !messages.empty();
}

bool Database::insertFile(const std::string &filename, int senderId, int recipientId, int channelId, const std::string &uuid)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "INSERT INTO files (filename, sender_id, recipient_id, channel_id, uuid) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the filename to the statement
    sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_STATIC);

    // Bind the sender_id to the statement
    sqlite3_bind_int(stmt, 2, senderId);

    // Bind the recipient_id or channel_id (check for one being non-null)
    if (recipientId != 0)
    {
        sqlite3_bind_int(stmt, 3, recipientId); // Bind recipient_id
        sqlite3_bind_null(stmt, 4);             // Null channel_id
    }
    else if (channelId != 0)
    {
        sqlite3_bind_null(stmt, 3);           // Null recipient_id
        sqlite3_bind_int(stmt, 4, channelId); // Bind channel_id
    }
    else
    {
        std::cerr << "Both recipient_id and channel_id cannot be zero" << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Bind the uuid to the statement
    sqlite3_bind_text(stmt, 5, uuid.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}

bool Database::getFileByUUID(const std::string &uuid, std::string &filename, int &recipientId, int &channelId)
{
    sqlite3 *db = pool.acquireConnection();
    const char *sql = "SELECT filename, recipient_id, channel_id FROM files WHERE uuid = ?;";
    sqlite3_stmt *stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        pool.releaseConnection(db);
        return false;
    }

    // Bind the uuid to the statement
    sqlite3_bind_text(stmt, 1, uuid.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW)
    {
        std::cerr << "Execution failed or no result found: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        pool.releaseConnection(db);
        return false;
    }

    filename = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    recipientId = sqlite3_column_int(stmt, 1);
    channelId = sqlite3_column_int(stmt, 2);

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
    pool.releaseConnection(db);
    return true;
}
