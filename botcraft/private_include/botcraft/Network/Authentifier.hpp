#pragma once
#include <nlohmann/json.hpp>

#include <string>

namespace Botcraft
{
    struct WebRequestResponse
    {
        nlohmann::json response;
        unsigned int status_code;
        std::string status_message;
    };

    class Authentifier
    {
    public:
        Authentifier();
        ~Authentifier();

        /// @brief Authentication using a legacy Mojang account (login + password).
        /// @param login Valid Mojang login
        /// @param password Valid Mojang password
        /// @return True if successfully authenticated, false otherwise
        const bool AuthMojang(const std::string& login, const std::string& password);

        /// @brief Authentication using a Microsoft account. If
        /// login is set, use it to identify the cached credentials.
        /// @param login Login is used as key to identify the credentials in cache file
        /// @return True if successfully authenticated, false otherwise
        const bool AuthMicrosoft(const std::string& login);

        // Join a server after encryption request arrived
        const bool JoinServer(const std::string& server_id, const std::vector<unsigned char>& shared_secret, const std::vector<unsigned char>& public_key) const;

        const std::string& GetPlayerDisplayName() const;

    private:
#ifdef USE_ENCRYPTION
        /// @brief Get the content of the whole cache file
        /// @return The content in JSON
        nlohmann::json GetCachedProfiles() const;

        /// @brief Try to find a cached account corresponding to login.
        /// Only one Microsoft account can be cached using an empty login.
        /// @param login Login of the account
        /// @return Cached credentials for the corresponding account
        nlohmann::json GetCachedCredentials(const std::string& login) const;

        /// @brief Extract the token, the name and the uuid from a server response
        /// @param response The json response sent by the auth server
        /// @return A tuple containing <the new token, the new name, the new uuid>, all empty if failed
        const std::tuple<std::string, std::string, std::string> ExtractMCFromResponse(const nlohmann::json& response) const;

        /// @brief Check whether or not a minecraft token is still valid
        /// @param token The token to check
        /// @return True if token is valid, false otherwise
        const bool IsMCTokenValid(const std::string& token) const;

        /// @brief Refresh an unvalid MC token
        /// @param token The token to refresh
        /// @return A tuple containing <the new token, the new name, the new uuid>, all empty if failed
        const std::tuple<std::string, std::string, std::string> RefreshMCToken(const std::string& token) const;

        /// @brief Check if a validity time is in the present or in the future
        /// @param t The expiration timestamp
        /// @return True if expired, false if valid
        const bool IsTokenExpired(const long long int& t) const;

        /// @brief Save a profiles list to cache file
        /// @param profiles A json object with logins as keys and cache credentials as values
        void WriteCacheFile(const nlohmann::json& profiles) const;

        /// @brief Update the cached MSA data for the given login
        /// @param login The login we want to update the data for
        /// @param access_token New access token
        /// @param refresh_token New refresh token
        /// @param expiration New expiration date
        void UpdateCachedMSA(const std::string& login, const std::string& access_token,
            const std::string& refresh_token, const long long int& expiration) const;

        /// @brief Update the cached MC token data for the given login
        /// @param login The login we want to update the data for
        /// @param mc_token New MC token
        /// @param expiration New token expiration date
        void UpdateCachedMCToken(const std::string& login, const std::string& mc_token,
        const long long int& expiration) const;

        /// @brief Update the cached MC profile data for the given login
        /// @param login The login we want to update the data for
        /// @param name New MC name
        /// @param id New MC uuid
        void UpdateCachedMCProfile(const std::string& login, const std::string& name,
            const std::string& id) const;

        /// @brief Update the cached MC data for the given login
        /// @param login The login we want to update the data for
        /// @param name New MC name
        /// @param id New MC uuid
        /// @param token New MC token
        void UpdateCachedMC(const std::string& login, const std::string& name,
            const std::string& id, const std::string& token);

        /// @brief Check if there is a saved credentials file and
        /// if the token is still valid. Refresh it if not.
        /// If file doesn't exist, launch auth device flow
        /// @param login Login used as key for the cached credentials
        /// @return The microsoft access token, empty if failed.
        const std::string GetMSAToken(const std::string& login) const;

        /// @brief Try to authenticate with microsoft account using device flow.
        /// Save the credentials to cached file if success.
        /// @return The microsoft access token, empty if failed.
        const std::string MSAAuthDeviceFlow(const std::string& login) const;

        /// @brief Try to get XBox Live token from Microsoft token.
        /// @param msa_token Microsoft access token
        /// @return XBL token, empty if failed.
        const std::string GetXBLToken(const std::string& msa_token) const;
                
        /// @brief Try to get XSTS token from XBL token.
        /// @param xbl_token XBL token
        /// @return Pair of {XSTS token, userhash}, empty if failed.
        const std::pair<std::string, std::string> GetXSTSToken(const std::string& xbl_token) const;

        /// @brief Try to get MC token from XSTS token and user hash.
        /// @param login Login used to store credentials in cache
        /// @param xsts_token XSTS Token
        /// @param user_hash User hash
        /// @return Minecraft token, empty if failed.
        const std::string GetMCToken(const std::string& login, 
            const std::string& xsts_token, const std::string& user_hash) const;

        /// @brief Try to get Minecraft profile from Minecraft token
        /// @param login Login used to store credentials in cache
        /// @param mc_token Minecraft token
        /// @return Pair of {MC UUID, MC name}, empty if failed.
        const std::pair<std::string, std::string> GetMCProfile(const std::string& login, 
            const std::string& mc_token) const;

        /// @brief Send a web request with ssl stuff
        /// @param host The host address
        /// @param raw_request The full request (header + content) as it should be sent
        /// @return A WebRequestResponse returned by the server
        const WebRequestResponse WebRequest(const std::string& host, const std::string& raw_request) const;

        /// @brief Send a POST request with ssl stuff
        /// @param host The host address (after https:// and before the first /)
        /// @param endpoint The endpoint (after the first /)
        /// @param content_type Data type
        /// @param accept Accept header value
        /// @param data Actual data to send
        /// @return A WebRequestResponse returned by the server
        const WebRequestResponse POSTRequest(const std::string& host, const std::string& endpoint,
            const std::string& content_type, const std::string& accept, const std::string& data) const;

        /// @brief Send a GET request with ssl stuff
        /// @param host The host address (after https:// and before the first /)
        /// @param endpoint The endpoint (after the first /)
        /// @param authorization Optional authorization header, only set if not empty
        /// @return A WebRequestResponse returned by the server
        const WebRequestResponse GETRequest(const std::string& host, const std::string& endpoint,
            const std::string& authorization = "") const;
#endif

    private:
        
        /// @brief Path to cache the credentials
        static const std::string cached_credentials_path;
                
        /// @brief Botcraft app ID for microsoft auth
        static const std::string botcraft_app_id;

        /// @brief Default cached credentials JSON
        static const nlohmann::json defaultCachedCredentials;
        
        std::string player_display_name;
        std::string mc_access_token;
        std::string mc_player_uuid;
		
    };
}
