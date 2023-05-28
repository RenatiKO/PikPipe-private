//
// Created by nikita on 24.05.23.
//

#ifndef PIPIK_PIPEALGO_H
#define PIPIK_PIPEALGO_H

#include "dxf_parser.h"
#include "eigen3/Eigen/Geometry"
#include "eigen3/Eigen/Eigen"
#include "yaml-cpp/yaml.h"
#include "cassert"
#include "string"

//enum Element {
//    SINK = 0,
//    SHOWER,
//    BATH,
//    //TOILET
//};
enum FittingType {
    TEE = 0,
    CROSSPIECE,
    TAP,
    REDUCTION,
    UNKNOWN
};


class FittingYaml {
public:
    FittingYaml(std::string n, std::string t) {
        name_ = n;
        type_ = t;
    }

    std::string name_, type_;
};


std::string enumToStrng(FittingType t);

//struct EnterPoint {
//    SimplePoint point_;
//    Element element_;
//};

class Fitting;

class Connector {
public:
    Connector();

    Connector(double diameter, Eigen::Quaternionf direction, bool is_out_);

    void init(int id, const Fitting &neighbour, const Fitting &home_fitting, const Eigen::Quaternionf &q);

    void rotate(const Eigen::Quaternionf &q);

    bool getUse() const;

    int getId() const;

    bool isOut() const;

    bool isInit() const;

    double get_diametr() const;

    Eigen::Quaternionf get_direction() const;

private:
    int id_;
    bool use_; // свободно или нет
    double diameter_; //метры
    Eigen::Quaternionf direction_;
    bool is_out;
    std::shared_ptr<Fitting> neighbour_;//
    std::shared_ptr<Fitting> home_fitting_;//
    double length;// от центра масс фтинга
};

class Fitting {
public:
    Fitting();

    Fitting(std::string name, FittingType type, std::vector<Connector> &connectors);

    void rotate(const Eigen::Quaternionf &q);

    double get_out_diameter() const;

    std::vector<Connector> get_ins() const;

    std::string get_name() const;

    FittingType get_type() const;

    Eigen::Quaternionf get_out_direction() const;

private:
    std::string name_;
    FittingType type_;
    std::vector<Connector> connectors_;
    int id_;
    SimplePoint point_;
};


struct YAMLPOINT {
    double x, y, z;
    int tag;
};

typedef std::vector<YAMLPOINT> YAMLPOINTS;

namespace YAML {
    bool addConnector(const Node &node, std::vector<Connector> &connectors);

    template<>
    struct convert<Fitting> {
        static bool decode(const Node &node, Fitting &rhs) {
            if (!node.IsMap()) {
                assert("is not map");
                return false;
            }
            FittingType type = FittingType::UNKNOWN;
            std::string fitting_name = node["name"].as<std::string>();
            std::vector<Connector> connectors_vec;
            if (node["type"].as<std::string>() == "tap") {
                type = FittingType::TAP;
            }
            if (node["type"].as<std::string>() == "tee") {
                type = FittingType::TEE;
            }
            if (node["type"].as<std::string>() == "crosspiece") {
                type = FittingType::CROSSPIECE;
            }
            Node connectors = node["connectors"];
            if (!connectors.IsSequence()) {
                assert("is not seq");
                return false;
            }
            for (const Node &con: connectors) {
                if (!addConnector(con, connectors_vec)) {
                    return false;
                }
            }
            for (const auto &d: connectors_vec) {
                std::cout << d.isOut() << " - " << d.get_diametr() << std::endl;
            }

            rhs = Fitting(fitting_name, type, connectors_vec);
            return true;
        }

        static Node encode(const Fitting &rhs) {
            return Node();
        }
    };

    template<>
    struct convert<FittingYaml> {
        static Node encode(const FittingYaml &f) {
            YAML::Node node;
            node["name"] = f.name_;
            node["type"] = f.type_;
            return node;
        }
    };

    template<>
    struct convert<YAMLPOINTS> {

        static Node encode(const YAMLPOINTS &points) {
            YAML::Node nodes;
            for (const auto &p: points) {
                YAML::Node node;
                node["x"] = p.x;
                node["y"] = p.y;
                node["z"] = p.z;
                node["tag"] = p.tag;
                nodes.push_back(node);
            }
            return nodes;
        }

        static bool decode (const Node &node, YAMLPOINTS &rhs) {
//            YAMLPOINTS points;
            if (!node.IsSequence()) {
                assert("!sequence");
            }
            std::cout << "YAMLPOINTS size == " << node.size() << std::endl;
            for (auto n : node) {
                if (!n.IsMap()) {
                    assert("!map");
                }
                YAMLPOINT p;
                p.x = n["x"].as<double>();
                p.y = n["y"].as<double>();
                p.z = n["z"].as<double>();
                p.tag = n["tag"].as<int>();
                rhs.push_back(p);
            }
            return true;
        }

    };

    template<>
    struct convert<PipesToFront> {

        static Node encode(const PipesToFront &points) {
            YAML::Node nodes;
            for (const auto &p: points) {
                YAML::Node node;
                YAML::Node point;
                YAML::Node n_point;
                point["x"] = p.point.x();
                point["y"] = p.point.y();
                point["z"] = p.point.z();
                node["point"] = point;

                n_point["x"] = p.next_p.x();
                n_point["y"] = p.next_p.y();
                n_point["z"] = p.next_p.z();
                node["next"] = n_point;

                node["diameter"] = p.diameter;
                node["id"] = p.id;
                nodes.push_back(node);
            }
            return nodes;
        }

//        static bool decode (const Node &node, PipesToFront &rhs) {
////            YAMLPOINTS points;
//            if (!node.IsSequence()) {
//                assert("!sequence");
//            }
//            std::cout << "YAMLPOINTS size == " << node.size() << std::endl;
//            for (auto n : node) {
//                if (!n.IsMap()) {
//                    assert("!map");
//                }
//                YAMLPOINT p;
//                p.x = n["x"].as<double>();
//                p.y = n["y"].as<double>();
//                p.z = n["z"].as<double>();
//                p.tag = n["tag"].as<int>();
//                rhs.push_back(p);
//            }
//            return true;
//        }

    };
}




bool loadYamlFittings(std::vector<Fitting> &fittings, const std::string &adr_fittings_list);


class PipeLine {

public:
    PipeLine();

    PipeLine(std::vector<PipeNode> &grahp, std::vector<Fitting> &available_fitings);

    YAML::Node makeFittingList();

private:
    std::vector<PipeNode> grahp_;
    Fitting dobor_110_45;
    Fitting dobor_50_45;
    std::map<int, std::vector<FittingYaml>> fitting_map;

    std::vector<Fitting> filterByDiameter(const PipeNode &node);

    std::vector<Fitting> fittings_;
};


#endif //PIPIK_PIPEALGO_H
