//
// Created by nikita on 24.05.23.
//

#ifndef PIPIK_PIPEALGO_H
#define PIPIK_PIPEALGO_H

#include <CGAL/Simple_cartesian.h>
#include "eigen3/Eigen/Geometry"
#include "eigen3/Eigen/Eigen"
#include "yaml-cpp/yaml.h"
#include "cassert"
#include "string"
#include "optional"

class CompositeFitting;

struct SimplePoint2d {
    double x;
    double y;
};

struct SimplePoint : public SimplePoint2d {
    double z;
};

struct SimpleLine {
    SimplePoint f;
    SimplePoint s;
};

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point3d;
using Point_2   = typename Kernel::Point_2;


typedef Kernel::Point_2 Point ;


struct PipeNode {
    Point3d point;
    int id = -1;
    double diameter = -1;
    std::vector<PipeNode*> in;
    PipeNode* out = nullptr;
    std::shared_ptr<CompositeFitting> cf = nullptr;
};

typedef std::vector<PipeNode> PipeGraph;

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

int radToDeg(double rad);


class Fitting;




class Connector {
public:
    Connector();

    Connector(double diameter, Eigen::Quaternionf direction, bool is_out_);

    void init(int id, const Fitting &neighbour, const Fitting &home_fitting, const Eigen::Quaternionf &q);

    void set_id(int id);

    void reducDiametr( double diametr);

    void rotate(const Eigen::Quaternionf &q);

    bool getUse() const;

    int getId() const;

    bool isOut() const;

    bool isInit() const;

    double get_diametr() const;

    Eigen::Quaternionf get_direction() const;
    bool operator < (const Connector&  r) const;
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

class BaseFitting{
public:
    virtual bool is_composite()const =0;
};

class Fitting : private BaseFitting{
public:
    Fitting();

    Fitting(std::string name, FittingType type, std::vector<Connector> &connectors);

    void rotate(const Eigen::Quaternionf &q);

    double get_out_diameter() const;

    std::vector<Connector> get_ins() const;

    Connector get_out () const;

    std::string get_name() const;

    FittingType get_type() const;

    Eigen::Quaternionf get_out_direction() const;

    bool is_composite() const;
    void setIdConnectors();
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
            if (node["type"].as<std::string>() == "reduction") {
                type = FittingType::REDUCTION;
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
                     rhs = Fitting(fitting_name, type, connectors_vec);
            rhs.setIdConnectors();
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

    };

    template<>
    struct convert<PipeGraph> {
        static bool decode(const Node &node, PipeGraph &rhs) {
            rhs.reserve(node.size());
            int order_check=0;
            for (const auto &n: node) {
                if (!n.IsMap()) {
                    assert("PipeNode isnt map");
                    return false;
                }
                PipeNode pipe_node;
                pipe_node.id = n["id"].as<int>();
                if(pipe_node.id!=order_check){
                    assert("ГРАФ НЕ Упорядочен оп ID!!!");
                }
                order_check++;
                pipe_node.diameter = n["diameter"].as<double>();
                pipe_node.out = nullptr;
                const auto& point_ = n["point"];
                if(!point_.IsMap()){
                    assert("point isnt map");
                    return false;
                }
                pipe_node.point = {point_["x"].as<double>(),point_["y"].as<double>(),point_["z"].as<double>()};
                rhs.push_back(std::move(pipe_node));
            }
            for (const auto &n: node) {
                auto& node_ = rhs[n["id"].as<int>()];
                int out_id = n["out"].as<int>();
                if (out_id == -1) {
                    node_.out = nullptr;
                } else {
                    node_.out = &rhs[out_id];
                }
                auto ins = n["in"];
                for(const auto& in : ins){
                    auto& in_ =  rhs[in.as<int>()];
                    node_.in.push_back(&in_);
                }
            }



            return true;
        }

        static Node encode(const PipeGraph &pipe_nodes) {
            YAML::Node nodes;
            for (const auto &pipe_node: pipe_nodes) {
                YAML::Node node;
                node["id"] = pipe_node.id;
                if (pipe_node.out != nullptr) {
                    node["out"] = pipe_node.out->id;
                } else {
                    node["out"] = -1;
                }
                YAML::Node point;
                point["x"] = pipe_node.point.x();
                point["y"] = pipe_node.point.y();
                point["z"] = pipe_node.point.z();
                node["point"] = point;
                YAML::Node ins;
                for (const auto &in: pipe_node.in) {
                    ins.push_back(in->id);
                }
                node["in"] = ins;
                node["diameter"] = pipe_node.diameter;
                nodes.push_back(node);
            }
            return nodes;
        }
    };

}


bool loadYamlFittings(std::vector<Fitting> &fittings, const std::string &adr_fittings_list);


// CompositeFitting
//______________________________________________________________________________

class CompositeFitting : public BaseFitting{
private:
    Fitting main_fitting_;
    std::map<Connector,std::vector<Fitting> > child_fittings_;
public:
    CompositeFitting();
    CompositeFitting(Fitting& main_fitting);
    std::optional<Connector> getConByQuat(Eigen::Quaternionf quat);
    void operator = (const CompositeFitting& r);
    bool addChildFitting(Fitting& child_fitting,  std::shared_ptr<Connector> connector);
    std::vector<const std::shared_ptr<Fitting>> getAllFitings() const;
    std::vector<std::shared_ptr<Connector>> getIns () const;
    std::vector<Connector> getCompositeConnectors() const;

    Connector getOut() const;
    int getDegree()const;
    bool is_composite() const;

};
//ToDo реализация
//______________________________________________________________________________



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



    std::vector<Fitting> fittings_;

    std::vector<CompositeFitting> filterByInOut(PipeNode& node);

    void filterByAngle(std::vector<CompositeFitting>& comp_fitt,const PipeNode& node);

    std::optional <Fitting> findAvailibleReduction(double diameter_in, double diameter_out) const;
    //ToDo реализация
    std::optional <Fitting> findAvailibleTap(double angle, double diameter) const;

};

#endif //PIPIK_PIPEALGO_H
