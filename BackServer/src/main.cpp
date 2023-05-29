#include <drogon/drogon.h>
#include <iostream>
#include "dxf_parser.h"
#include "dxfrw/libdxfrw.h"
#include <PipeAlgo/PipeAlgo.h>
#include <yaml-cpp/yaml.h>

using namespace drogon;

int main()
{
    app().registerHandler(
                "/",
                [](const HttpRequestPtr &,
                std::function<void(const HttpResponsePtr &)> &&callback) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("Hello, World!");
        callback(resp);
    });

    app().registerHandler(
                "/calc",
                [](const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback) {
        MultiPartParser fileUpload;
        std::cout << "sdbcfkfjblj" << std::endl;
        if (fileUpload.parse(req) != 0)
        {
            std::cout << fileUpload.getFiles()[0].getFileName() << std::endl;
            //                callback(resp);
            return;
        }
        std::cout << fileUpload.getFiles()[0].getFileName() << std::endl;

        YAMLPOINTS points;
        for (auto p : fileUpload.getParameters()) {
            std::cout << p.first << " - " << p.second << std::endl;
            YAML::Node dots = YAML::Load(p.second);
            points = dots.as<YAMLPOINTS>();
        }

        for (auto p : points) {
            std::cout << p.x << " - " << p.y << " - " << p.z << " - " << p.tag << std::endl;
        }

        auto &file = fileUpload.getFiles()[0];
        auto md5 = file.getMd5();
        //            resp->setBody(
        //                "The server has calculated the file's MD5 hash to be " + md5);
        file.save();
        LOG_INFO << "The uploaded file has been saved to the ./uploads "
                    "directory";
        DxfParser parser;
        dxfRW dxf(std::string("./uploads/" + fileUpload.getFiles()[0].getFileName()).c_str());

        if (!dxf.read(&parser, false)) {
            std::cerr << "drawing.dxf could not be opened.\n";
        }
        parser.wallSkeletonWithHoles();

        for (auto p : points) {
            parser.addVertexToGraph({p.x, p.y, p.z});
            if (p.tag != 0) {
                parser.addMission({p.x, p.y, p.z}, SanType(p.tag));
            } else {
                parser.setRootOfGraph({p.x, p.y, p.z});
            }
        }

        auto res = parser.graphSearch();

        std::vector<Fitting> fittings;
        loadYamlFittings(fittings, "../../data/avalible_fittings.yaml");
        auto graph = parser.getPipeNodes();
        PipeLine pipe_line(graph, fittings);
        YAML::Node result = pipe_line.makeFittingList();
        std::cout << result << std::endl;
        std::ofstream out("fittings_list.yaml");
        out << result;
        out.close();

        auto front = parser.getFrontNodes();
        YAML::Node front_node(front);

//        std::cout << front_node << std::endl;

        YAML::Emitter emitter1;
          emitter1 << YAML::DoubleQuoted << YAML::Flow << front_node;
          std::string out1(emitter1.c_str());
//          std::cout << "Output without BeginSeq:\n" << out1 << '\n';

          YAML::Emitter emitter2;
            emitter2 << YAML::DoubleQuoted << YAML::Flow << result;
            std::string out2(emitter2.c_str());
//            std::cout << "Output without BeginSeq:\n" << out1 << '\n';

//        for (auto fn : front_node) {
//            fn.
//        }
//        Json::Value json;
//        json["ok"] = 5.;




////        json["ok"] = false;
//        Json::Value json;
        Json::Value json;
        json[0] = out1;
        json[1] = out2;
        auto resp = HttpResponse::newHttpJsonResponse(json);
//        resp->setBody(out1);
        callback(resp);
    },
    {Post});

    LOG_INFO << "Server running on 45.147.179.102:8888";
    //    app().registerPostHandlingAdvice(
    //            [](const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
    //                //LOG_DEBUG << "postHandling1";
    //                resp->addHeader("Access-Control-Allow-Origin", "*");
    //            });
    app()
            .setClientMaxBodySize(20 * 2000 * 2000)
            .setUploadPath("./uploads")
            .addListener("45.147.179.102", 8888)
            .registerPostHandlingAdvice(
                [](const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
        //LOG_DEBUG << "postHandling1";
        resp->addHeader("Access-Control-Allow-Origin", "*");
    }).run();

}
