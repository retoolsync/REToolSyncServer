#include "cpp-httplib/httplib.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <cinttypes>
#include <mutex>
#include "json.hpp"

#define DEFAULT_HOST "0.0.0.0"
#define DEFAULT_PORT 8282

struct Cursor
{
    // generic metadata
    uint64_t session = -1;
    std::string toolid = "x64dbg";
#ifdef _WIN64
    std::string architecture = "x64";
#else
    std::string architecture = "x86";
#endif
    std::string cursorid; // string that describes which cursor this is (dump/disassembly/decompiler/etc)

    // actual information
    uint64_t va = -1;
    uint32_t rva = -1;
    uint64_t fileoffset = -1;

    // metadata
    std::string filepath;
    std::string sha1;
    uint32_t TimeDateStamp = -1;
    uint64_t imagebase = -1; // should this be the currently loaded imagebase (probably yes) or the one in the header?
    uint32_t imagesize = -1;

    void dump() const
    {
        puts(serialize(2).c_str());
    }

    static std::string toHex(uint64_t value)
    {
        char text[32];
        sprintf_s(text, "0x%llx", value);
        return text;
    }

    static uint64_t fromHex(const std::string& text)
    {
        uint64_t value = 0;
        if (sscanf_s(text.c_str(), "0x%" SCNx64, &value) != 1)
            throw std::invalid_argument("fromHex failed");
        return value;
    }

    static uint64_t fromDec(const std::string& text)
    {
        uint64_t value = 0;
        if (sscanf_s(text.c_str(), "%" SCNu64, &value) != 1)
            throw std::invalid_argument("fromDec failed");
        return value;
    }

    std::string serialize(int indent = -1) const
    {
        nlohmann::json j;
        j["session"] = std::to_string(session);
        j["toolid"] = toolid;
        j["architecture"] = architecture;
        j["cursorid"] = cursorid;
        j["va"] = toHex(va);
        j["rva"] = toHex(rva);
        j["fileoffset"] = toHex(fileoffset);
        j["filepath"] = filepath;
        j["sha1"] = sha1;
        j["TimeDateStamp"] = toHex(TimeDateStamp);
        j["imagebase"] = toHex(imagebase);
        j["imagesize"] = toHex(imagesize);
        return j.dump(indent);
    }

    static bool deserialize(const nlohmann::json::value_type& j, Cursor& c)
    {
        try
        {
            c = Cursor();
            c.session = fromDec(j["session"]);
            c.toolid = j["toolid"];
            c.architecture = j["architecture"];
            c.cursorid = j["cursorid"];
            c.va = fromHex(j["va"]);
            c.rva = (uint32_t)fromHex(j["rva"]);
            c.fileoffset = fromHex(j["fileoffset"]);
            c.filepath = j["filepath"];
            c.sha1 = j["sha1"];
            c.TimeDateStamp = (uint32_t)fromHex(j["TimeDateStamp"]);
            c.imagebase = fromHex(j["imagebase"]);
            c.imagesize = (uint32_t)fromHex(j["imagesize"]);
        }
        catch (const nlohmann::json::exception & x)
        {
            return false;
        }
        catch (const std::invalid_argument & x)
        {
            return false;
        }
        return true;
    }

    static bool deserialize(const std::string& json, Cursor& c)
    {
        auto j = nlohmann::json::parse(json);
        if (!j.is_object())
            return false;
        return deserialize(j, c);
    }

    static bool deserialize(const std::string& json, std::vector<Cursor>& cs)
    {
        auto j = nlohmann::json::parse(json);
        if (!j.is_array())
            return false;
        cs.reserve(j.size());
        for (const auto& item : j)
        {
            Cursor c;
            if (!deserialize(item, c))
                return false;
            cs.push_back(c);
        }
        return true;
    }
};

struct SessionCursors
{
    time_t lastHeartbeat = 0;
    std::unordered_map<std::string, Cursor> cursors;
};

using Universe = std::unordered_map<uint64_t, SessionCursors>;

static std::unordered_map<std::string, Universe> cursors;
static std::mutex mutCursors;

static bool serve(const char* host, int port)
{
    using namespace httplib;

    Server svr;
    svr.Post("/cursor/([^/]+)", [](const Request& req, Response& res)
    {
        auto universe = req.matches[1].str();

        std::vector<Cursor> cs;
        if (!Cursor::deserialize(req.body, cs))
        {
            res.status = 400;
            res.body = "failed to deserialize request";
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutCursors);
            for (const auto& c : cs)
            {
                printf("session %llu\n", c.session);
                auto& session = cursors[universe][c.session];
                session.cursors[c.cursorid] = c;
                session.lastHeartbeat = time(nullptr);
            }
        }

        res.body = "nice!";
    });

    svr.Post("/session/([^/]+)", [](const Request& req, Response& res)
    {
        auto universe = req.matches[1].str();

        try
        {
            auto session = Cursor::fromDec(req.get_param_value("session"));
            std::lock_guard<std::mutex> lock(mutCursors);
            auto universeItr = cursors.find(universe);
            if (universeItr == cursors.end())
            {
                res.status = 400;
                res.body = "invalid session parameter";
                return;
            }
            auto sessionItr = universeItr->second.find(session);
            if (sessionItr == universeItr->second.end())
            {
                res.status = 400;
                res.body = "invalid session parameter";
                return;
            }
            sessionItr->second.lastHeartbeat = time(nullptr);
            printf("heartbeat session %llu\n", session);
        }
        catch (const std::invalid_argument&)
        {
            res.status = 400;
            res.body = "invalid session parameter";
            return;
        }
    });

    svr.Get("/cursor/([^/]+)", [](const Request& req, Response& res)
    {
        //TODO: implement filter parameters based on sha1/filename/etc

        req.get_param_value("");
        auto universe = req.matches[1].str();
        auto universeItr = cursors.find(universe);
        if (universeItr == cursors.end())
        {
            res.status = 404;
            res.body = "universe not found";
            return;
        }

        std::vector<Cursor> cs;
        {
            auto now = time(nullptr);
            std::lock_guard<std::mutex> lock(mutCursors);
            for (auto sessionItr = universeItr->second.begin(); sessionItr != universeItr->second.end(); ++sessionItr)
            {
                auto lastHeartbeat = difftime(now, sessionItr->second.lastHeartbeat);
                if (lastHeartbeat > 5.0f)
                {
                    printf("session %llu expired, deleting...\n", sessionItr->first);
                    sessionItr = universeItr->second.erase(sessionItr);
                    if (sessionItr == universeItr->second.end())
                        break;
                }

                for (const auto& cursorItr : sessionItr->second.cursors)
                {
                    cs.push_back(cursorItr.second);
                }
            }
        }

        res.set_header("Content-Type", "application/json");
        res.body = "[";
        std::string cursorJson;
        for (size_t i = 0; i < cs.size(); i++)
        {
            if (i)
                res.body += ",";
            res.body += cs[i].serialize();
        }
        res.body += "]";
    });

    svr.set_logger([](const Request& req, const Response& res)
    {
        printf("%s '%s': %s\n", req.method.c_str(), req.path.c_str(), req.body.c_str());
    });

    printf("serving on %s:%d\n", host, port);
    return svr.listen(host, port);
}

static int usage()
{
    fprintf(stderr, "usage: REToolSyncServer [--host %s] [--port %d]", DEFAULT_HOST, DEFAULT_PORT);
    return 1;
}

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
            return usage();

    auto getArg = [argc, argv](const char* name) -> const char*
    {
        for (int i = 1; i < argc - 1; i++)
            if (strcmp(argv[i], name) == 0)
                return argv[i + 1];
        return nullptr;
    };


    auto port = DEFAULT_PORT;
    {
        auto portArg = getArg("--port");
        if (portArg)
            port = atoi(portArg);
        if (port < 1 || port > 65535)
            return usage();
    }

    auto host = DEFAULT_HOST;
    {
        auto hostArg = getArg("--host");
        if (hostArg)
            host = hostArg;
    }

    return serve(host, port) ? 0 : 1;
}