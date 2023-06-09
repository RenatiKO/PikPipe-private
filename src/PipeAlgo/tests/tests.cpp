//
// Created by Никита Мамонтов on 04.06.2023.
//

#include "include/PipeAlgo.h"

#define BOOST_TEST_MODULE first_test
//#define BOOST_TEST_NO_MAIN
#include <boost/test/included/unit_test.hpp>
#include "yaml-cpp/yaml.h"
#include <filesystem>

namespace fs = std::filesystem;

std::filesystem::path getPathToPikPipe() {
    fs::path path = fs::current_path();
    auto pik_pipe_it = std::find(path.begin(), path.end(), "PikPipe");
    fs::path dir;
    for (auto it = path.begin(); it != pik_pipe_it; it++) {
        dir /= *it;
    }
    dir /= fs::path("PikPipe");
    return dir;
}

std::pair<std::filesystem::path, std::filesystem::path> getPathToGraphs(int i) {
    fs::path dir = getPathToPikPipe();
    dir /= fs::path("src") / fs::path("PipeAlgo") / fs::path("tests") /
           fs::path("tests_yaml_graphs");

    std::string test_s = "test_fitting_" + std::to_string(i) + ".yaml";
    std::string r_s = "test_result_" + std::to_string(i) + ".yaml";

    return std::make_pair(fs::path(dir / test_s), fs::path(dir / r_s));
}


const fs::path path_fit(getPathToPikPipe() /= fs::path("data") / fs::path("avalible_fittings.yaml"));


bool get_res(int i) {
    auto test_path = getPathToGraphs(i);
    std::vector<Fitting> fittings;
    YAML::Node result = YAML::LoadFile(test_path.second);
    PipeGraph graph = YAML::LoadFile(test_path.first).as<PipeGraph>();
    loadYamlFittings(fittings, path_fit);
    PipeLine pipe_line(graph, fittings);
    YAML::Node try_ = pipe_line.makeFittingList();

    bool bool_result = true;
    if (try_.size() != result.size()) {
        std::cout << "\n НЕ РАВНЫЙ РАЗМЕР ВЕРНУЛО(" << try_.size() << ")" << "НАДО (" << result.size() << ")"
                  << std::endl;
        std::cout<<try_<<std::endl;
        bool_result = false;
    }
    for (size_t r = 0; r < result.size(); r++) {
        YAML::Node r_ = result[r];
        std::stringstream s_r;
        for (size_t t = 0; t < try_.size(); t++) {
            YAML::Node t_ = try_[t];
            if (t_["Node"].as<int>() == r_["Node"].as<int>()) {
                std::stringstream s_t;
                s_r << r_;
                s_t << t_;
                if (s_r.str() != s_t.str()) {
                    std::cout << "\n НУЖНО_________________" << std::endl;
                    std::cout << r_ << std::endl;
                    std::cout << "ВЕРНУЛО_________________" << std::endl;
                    std::cout << t_ << std::endl;
                    bool_result = false;
                }
                break;
            }
        }
    }
    return bool_result;
}

BOOST_AUTO_TEST_CASE(test1)
{
    BOOST_TEST(get_res(1));
}

BOOST_AUTO_TEST_CASE(test2)
{
    BOOST_TEST(get_res(2));
}

BOOST_AUTO_TEST_CASE(test3)
{
    auto test_path = getPathToGraphs(1);
    YAML::Node result = YAML::LoadFile(test_path.second);
    PipeGraph graph = YAML::LoadFile(test_path.first).as<PipeGraph>();
    std::vector<Fitting> fittings;
    loadYamlFittings(fittings, path_fit);
    for (const auto &f: fittings) {
        if (f.get_name() == "110x110x87") {
            Eigen::Quaternionf out_q = f.get_out_direction();
            auto in_q = f.get_ins()[0].get_direction();
            float angle = out_q.angularDistance(in_q);
            BOOST_TEST(radToDeg(angle) == 180);
            in_q = f.get_ins()[1].get_direction();
            angle = out_q.angularDistance(in_q);
            BOOST_TEST(radToDeg(angle) == 93);
        }
        if (f.get_name() == "50x50x45" && f.get_type() == TEE) {
            Eigen::Quaternionf out_q = f.get_out_direction();
            auto in_q = f.get_ins()[0].get_direction();
            float angle = out_q.angularDistance(in_q);
            BOOST_TEST(radToDeg(angle) == 180);
            in_q = f.get_ins()[1].get_direction();
            angle = out_q.angularDistance(in_q);
            BOOST_TEST(radToDeg(angle) == 45);
        }
    }

}