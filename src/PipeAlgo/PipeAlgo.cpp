#include "PipeAlgo/PipeAlgo.h"

#include <utility>

Connector::Connector(double diameter, Eigen::Quaternionf direction, bool is_out_) : id_(0), use_(false),
                                                                                    diameter_(diameter),
                                                                                    direction_(std::move(direction)) {
    this->is_out = is_out_;
    neighbour_ = NULL;
    home_fitting_ = NULL;
}

Connector::Connector() {
    id_ = 0;
    is_out = false;
    use_ = false;
    neighbour_ = NULL;
    home_fitting_ = NULL;
}

void Connector::init(int id, const Fitting &neighbour, const Fitting &home_fitting, const Eigen::Quaternionf &q) {
    id_ = id;
    neighbour_ = std::make_shared<Fitting>(neighbour);
    home_fitting_ = std::make_shared<Fitting>(home_fitting);
    direction_ = direction_ * q;
}

void Connector::rotate(const Eigen::Quaternionf &q) {
    direction_ = direction_ * q;
}

double Connector::get_diametr() const {
    return diameter_;
}

Eigen::Quaternionf Connector::get_direction() const {
    return direction_;
}

int Connector::getId() const { return id_; }

bool Connector::getUse() const { return use_; }

bool Connector::isOut() const { return is_out; }

bool Connector::isInit() const { return id_; }


Fitting::Fitting(std::string name, FittingType type, std::vector<Connector> &connectors) : name_(name), type_(type),
                                                                                           connectors_(std::move(
                                                                                                   connectors)),
                                                                                           id_(0) {
    point_.x = 0.;
    point_.y = 0.;
    point_.z = 0.;
}

Fitting::Fitting() {
    type_ = FittingType::UNKNOWN;
    id_ = 0;
    point_.x = 0.;
    point_.y = 0.;
    point_.z = 0.;
}

std::string Fitting::get_name() const { return name_; }

Eigen::Quaternionf Fitting::get_out_direction() const {
    for (const auto &c: connectors_) {
        if (c.isOut()) {
            return c.get_direction();
        }
    }
    assert("Bad fitting");
    return Eigen::Quaternionf();
}

double Fitting::get_out_diameter() const {
    for (const auto &con: connectors_) {
        if (con.isOut()) {
            return con.get_diametr();
        }
    }
    return 0;
}

FittingType Fitting::get_type() const { return type_; }

std::vector<Connector> Fitting::get_ins() const {

    std::vector<Connector> res;
    for (const auto &con: connectors_) {
        if (!con.isOut()) {
            res.push_back(con);
        }
    }
    return res;
}

void Fitting::rotate(const Eigen::Quaternionf &q) {
    for (auto &con: connectors_) {
        con.rotate(q);
    }
}

PipeLine::PipeLine(std::vector<PipeNode> &grahp, std::vector<Fitting> &available_fitings) {
    grahp_ = grahp;
    fittings_ = available_fitings;

    for (const auto &f: fittings_) {
        if (f.get_name() == "50X45") {
            dobor_50_45 = f;
        }
        if (f.get_name() == "110X45") {
            dobor_110_45 = f;
        }
    }
}

std::vector<Fitting> PipeLine::filterByDiameter(const PipeNode &node) {
    std::vector<Fitting> filter_fit_out;
    std::vector<Fitting> filter_fit_out_in;
    // фильтрация по диаметру выхода
    for (const auto &fit: fittings_) {
        if (node.out->diameter == fit.get_out_diameter()) {
            filter_fit_out.push_back(fit);
        }
    }
    for (const auto &fit: filter_fit_out) {
        auto ins = fit.get_ins();
        if (ins.size() == node.in.size()) {
            std::vector<double> node_ins;
            for (const auto &n: node.in) {
                node_ins.push_back(n->diameter);
            }
            std::vector<double> fit_ins;
            for (const auto &f: fit.get_ins()) {
                fit_ins.push_back(f.get_diametr());
            }
            std::sort(node_ins.begin(), node_ins.end());
            std::sort(fit_ins.begin(), fit_ins.end());
            if (node_ins == fit_ins) {
                filter_fit_out_in.push_back(fit);
            }
        }
    }
    return filter_fit_out_in;
}


YAML::Node PipeLine::makeFittingList() {
    for (const PipeNode &node: grahp_) {
        auto out = node.out;
        if (out == NULL && node.in.empty()) {
            std::cout << "Node = " << node.id << " is empty" << std::endl;
            continue;
        }

        // стояк
        if (node.out == NULL) {
            std::vector<double> node_in_dia;
            for (const auto &in: node.in) {
                node_in_dia.push_back(in->diameter);
            }
            for (const auto &f: fittings_) {
                auto out_q = f.get_out_direction();
                std::vector<double> fit_in_dia;
                for (const auto &in_fit: f.get_ins()) {
                    auto in_q = in_fit.get_direction();
                    float angle = out_q.angularDistance(in_q);
                    std::cout << in_fit.isOut() << " - " << f.get_name() << " УГООЛ стояк между кватами = " << angle
                              << std::endl;
                    if ((angle <= (M_PI + M_PI / 6) && angle >= (M_PI - M_PI / 6))) {
                        fit_in_dia.push_back(in_fit.get_diametr());
                    }
                }
                if (node_in_dia == fit_in_dia) {
                    fitting_map[node.id].emplace_back(f.get_name(), enumToStrng(f.get_type()));
                    break;
                }
            }
        }

            // начало трассы
        else if (node.in.empty() && !(out == NULL)) {
            double out_dia = out->diameter;
            if (out_dia == 50.0) {
                fitting_map[node.id].emplace_back(dobor_50_45.get_name(), enumToStrng(dobor_50_45.get_type()));
                fitting_map[node.id].emplace_back(dobor_50_45.get_name(), enumToStrng(dobor_50_45.get_type()));
            }
        } else {

            std::vector<Fitting> filter_fit_out_in = filterByDiameter(node);
            auto outPoint = node.out->point;
            auto nodePoint = node.point;
            Eigen::Vector3f out_vector(outPoint.x() - nodePoint.x(), outPoint.y() - nodePoint.y(),
                                       outPoint.z() - nodePoint.z());
            Eigen::Quaternionf q_out;
            q_out.setFromTwoVectors(Eigen::Vector3f::UnitX(), out_vector);
            for (const auto &in: node.in) {
                auto inPoint = in->point;
                Eigen::Vector3f in_vector(inPoint.x() - inPoint.x(), inPoint.y() - inPoint.y(),
                                          inPoint.z() - inPoint.z());
                Eigen::Quaternionf q_in;
                q_in.setFromTwoVectors(Eigen::Vector3f::UnitX(), in_vector);
                double angle = q_out.angularDistance(q_in);
                std::cout << in->id << " - " << out->id << " УГООЛ рег между кватами = " << angle << std::endl;
                if (!(angle <= (M_PI + M_PI / 6) && angle >= (M_PI - M_PI / 6))) {
                    if (in->diameter == 110.0) {
                        fitting_map[node.id].emplace_back(dobor_110_45.get_name(),
                                                          enumToStrng(dobor_110_45.get_type()));
                    }
                    if (in->diameter == 50.0) {
                        fitting_map[node.id].emplace_back(dobor_50_45.get_name(), enumToStrng(dobor_50_45.get_type()));
                    }
                }
            }
            if (!filter_fit_out_in.empty()) {
                fitting_map[node.id].emplace_back(filter_fit_out_in.front().get_name(),
                                                  enumToStrng(filter_fit_out_in.front().get_type()));
            }
        }
    }
    YAML::Node seq;
    for (const auto &[k, v]: fitting_map) {
        YAML::Node map_node;
        YAML::Node fitings_seq;
        for (const auto &f: v) {
            fitings_seq.push_back(f);
        }
        map_node["Node"] = k;
        map_node["fittings"] = fitings_seq;
        seq.push_back(map_node);
    }
    return seq;
}


bool loadYamlFittings(std::vector<Fitting> &fittings, const std::string &adr_fittings_list) {
    YAML::Node fittings_node = YAML::LoadFile(adr_fittings_list);
    if (!fittings_node.IsSequence()) {
        assert("Fittings node is not seq");
        return false;
    }
    for (const YAML::Node &fit: fittings_node) {
        fittings.push_back(fit.as<Fitting>());
    }
    return true;
}

std::string enumToStrng(FittingType t) {
    switch (t) {
        case 0:
            return "TEE";
        case 1:
            return "CROSSPIECE";
        case 2:
            return "TAP";
        case 3:
            return "REDUCTION";
        case 4:
            return "UNKNOWN";
        default:
            return "UNKNOWN";
    }
}


bool YAML::addConnector(const Node &node, std::vector<Connector> &connectors) {
    std::cout << "конектор НА ВХОДЕ " << node["out"].as<int>() << std::endl;
    bool out = node["out"].as<int>();
    if (!node.IsMap()) {
        assert("is not map");
        return false;
    }
    Node angles = node["angles"];
    if (!angles.IsSequence()) {
        assert("is not seq");
        return false;
    }
    Eigen::AngleAxisf dir(0, Eigen::Vector3f::UnitX());
    for (const Node &angle: angles) {
        if (!angle.IsMap()) {
            assert("is not map");
            return false;
        }
        float radian_angle = angle["angle"].as<float>() * static_cast<float>((M_PI / 180));
        char axis = angle["axis"].as<char>();
        switch (axis) {
            case 'X':
                dir = dir * Eigen::AngleAxisf(radian_angle, Eigen::Vector3f::UnitX());
                break;
            case 'Y':
                dir = dir * Eigen::AngleAxisf(radian_angle, Eigen::Vector3f::UnitY());
                break;
            case 'Z':
                dir = dir * Eigen::AngleAxisf(radian_angle, Eigen::Vector3f::UnitZ());
                break;
            default:
                assert("is not X Y Z");
                return false;
        }
    };
    connectors.emplace_back(node["diameter"].as<double>(), Eigen::Quaternionf(dir), out);
    return true;
}












