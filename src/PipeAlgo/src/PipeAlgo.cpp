#include "include/PipeAlgo.h"

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

void Connector::reducDiametr(double diametr) {
    diameter_ = diametr;
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

void Connector::set_id(int id) {
    id_ = id;
}

Eigen::Quaternionf Connector::get_direction() const {
    return direction_;
}

int Connector::getId() const { return id_; }

bool Connector::getUse() const { return use_; }

bool Connector::isOut() const { return is_out; }

bool Connector::isInit() const { return id_; }


bool Connector::operator<(const Connector &r) const {
    return id_ < r.id_;
}

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

int radToDeg(double rad) {
    return std::ceil(rad * 180 / M_PI);
}

std::string Fitting::get_name() const { return name_; }

bool Fitting::is_composite() const { return false; }

Connector Fitting::get_out() const {
    for (const auto &c: connectors_) {
        if (c.isOut()) {
            return c;
        }
    }
}


void Fitting::setIdConnectors() {
    int id = 1;
    for (auto &c: connectors_) {
        c.set_id(id);
        id++;
    }
}

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
    grahp_ = std::move(grahp);


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


std::vector<CompositeFitting> PipeLine::filterByInOut(PipeNode &node) {
    std::vector<CompositeFitting> result;

    std::vector<Fitting> filter_fit_out;
    std::vector<Fitting> filter_fit_out_in;
    // фильтрация по диаметру выхода
    double check_d = 0;
    if (node.out->cf != nullptr) {
        Eigen::Vector3f origin_point(node.out->point.x(), node.out->point.y(), node.out->point.z());
        Eigen::Vector3f direction_point(node.point.x(), node.point.y(), node.point.z());
        Eigen::Quaternionf quat;
        Eigen::Quaternionf null_quat(Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX()));
        direction_point -= origin_point;

        Eigen::Vector3f prev_direction_point(node.out->out->point.x(),node.out->out->point.y(),node.out->out->point.z());
        prev_direction_point-=origin_point;
        quat.setFromTwoVectors(prev_direction_point, direction_point);
        auto cf_n = node.out->cf->getConByQuat(null_quat * quat);
        check_d = cf_n->get_diametr();
    } else {
        check_d = node.out->diameter;
    }


    for (const auto &fit: fittings_) {

        if (check_d == fit.get_out_diameter() && fit.get_type() != REDUCTION) {
            filter_fit_out.push_back(fit);
        }
    }

    std::vector<double> node_ins;
    for (const auto &n: node.in) {

        node_ins.push_back(n->diameter);
    }
    std::sort(node_ins.begin(), node_ins.end());

    for (auto &fit: filter_fit_out) {
        auto ins = fit.get_ins();
        if (ins.size() == node.in.size()) {
            std::vector<std::shared_ptr<Connector>> fit_ins;
            for (auto &f: fit.get_ins()) {
                fit_ins.push_back(std::make_shared<Connector>(f));
            }

            std::sort(fit_ins.begin(), fit_ins.end(), [](const auto lhs, const auto rhs) {
                return lhs->get_diametr() < rhs->get_diametr();
            });

            CompositeFitting cf(fit);
            bool flag = true;
            for (size_t con = 0, ins_n = 0; con < fit_ins.size(); con++, ins_n++) {
                auto &connector = fit_ins[con];
                auto node_in = node_ins[ins_n];
                if (node_in != connector->get_diametr()) {
                    auto reduction = findAvailibleReduction(node_in, connector->get_diametr());
                    if (reduction) {
                        cf.addChildFitting(reduction.value(), connector);
                        result.push_back(cf);
                    } else {
                        flag = false;
                        break;
                    }
                }
            }
            if (flag) {
                result.push_back(cf);
            }
        }
    }
    std::sort(result.begin(), result.end(), [](const auto lhs, const auto rhs) {
        return lhs.getDegree() < rhs.getDegree();
    });
    return result;
}

void PipeLine::filterByAngle(std::vector<CompositeFitting> &comp_fitts, const PipeNode &node) {
    if (node.out == nullptr) {
        assert("РАСЧЕТ СТОЯКА");
    }
    Eigen::Quaternionf null_quat(Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX()));
    std::vector<Eigen::Quaternionf> ins_node_quats;
    Eigen::Vector3f self_node(node.point.x(), node.point.y(), node.point.z());
    Eigen::Vector3f out_node(node.out->point.x(), node.out->point.y(), node.out->point.z());
    out_node -= self_node;
    double step = 15;

    for (const auto &n: node.in) {
        Eigen::Vector3f in_node(n->point.x(), n->point.y(), n->point.z());
        in_node -= self_node;
        Eigen::Quaternionf q_in;
        q_in.setFromTwoVectors(out_node, in_node);
        ins_node_quats.push_back(null_quat * q_in);
    }

    std::sort(ins_node_quats.begin(), ins_node_quats.end(),
              [&null_quat](const Eigen::Quaternionf &lhs, const Eigen::Quaternionf &rhs) {
                  return null_quat.angularDistance(lhs) < null_quat.angularDistance(rhs);
              });

    std::vector<CompositeFitting> result;
    for (CompositeFitting &cf: comp_fitts) {
        if (cf.getIns().size() != ins_node_quats.size()) {
            assert("неравный размер фитинга и ноды");
        }


        bool add = true;
        for (size_t c = 0, n = 0; c < cf.getIns().size(); c++, n++) {
            auto con = cf.getIns()[c];
            auto &node_con = ins_node_quats[n];
            double сon_angular = radToDeg(null_quat.angularDistance(con->get_direction()));
            double node_angular = radToDeg(null_quat.angularDistance(ins_node_quats[n]));
            double dif = std::abs(сon_angular - node_angular);
            int counter = 0;
            if (dif < step &&
                !((180 - step) < сon_angular && сon_angular < (180 + step))) {
                add = false;
                break;
            }
            while (dif > step) {
                if (counter > 3) {
                    break;
                    add = false;
                }
                auto availible_tap = findAvailibleTap(dif, con->get_diametr());
                if (availible_tap) {
                    cf.addChildFitting(availible_tap.value(), con);
                    сon_angular = radToDeg(null_quat.angularDistance(con->get_direction()));
                    dif = std::abs(сon_angular - node_angular);
                    counter++;
                }
            }
        }
        if (add) {
            result.push_back(cf);
        }
    }
    comp_fitts = result;
    std::sort(comp_fitts.begin(), comp_fitts.end(), [](CompositeFitting &lhs, CompositeFitting &rhs) {
        return lhs.getDegree() < rhs.getDegree();
    });

}


std::optional<Fitting> PipeLine::findAvailibleReduction(double diameter_in, double diameter_out) const {
    for (const auto &fit: fittings_) {
        if (fit.get_type() == FittingType::REDUCTION) {
            if (fit.get_out().get_diametr() == diameter_out && fit.get_ins().front().get_diametr() == diameter_in) {
                return fit;
            }
        }
    }
    return std::optional<Fitting>();
}

std::optional<Fitting> PipeLine::findAvailibleTap(double angle, double diameter) const {
    Fitting res;
    bool find = false;
    double min_diff = angle;
    for (const auto &fit: fittings_) {
        if (fit.get_type() == FittingType::TAP && fit.get_out_diameter() == diameter) {
            Eigen::Quaternionf out = fit.get_out_direction();
            Eigen::Quaternionf in = fit.get_ins().front().get_direction();
            double tap_angle = out.angularDistance(in);
            double tap_angle_dif = std::abs(tap_angle - angle);
            if (min_diff >= tap_angle_dif) {
                res = fit;
                find = true;
                min_diff = tap_angle_dif;
            }
        }
    }
    return find ? res : std::optional<Fitting>();
}

YAML::Node PipeLine::makeFittingList() {
    for (PipeNode &node: grahp_) {
        PipeNode *out = node.out;
        //проверяем граф
        if (out == nullptr && node.in.empty()) {
            continue;
        }

        // стояк
        if (node.out == nullptr) {
            std::vector<double> node_in_dia;
            for (const auto &in: node.in) {
                node_in_dia.push_back(in->diameter);
            }
            for (const auto &f: fittings_) {
                Eigen::Quaternionf out_q = f.get_out_direction();
                std::vector<double> fit_in_dia;
                for (const auto &in_fit: f.get_ins()) {
                    auto in_q = in_fit.get_direction();
                    float angle = out_q.angularDistance(in_q);
                    if (radToDeg(angle) != 180) {
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
        else if (node.in.empty() && out != nullptr) {
            double out_dia = out->diameter;
            if (out_dia == 50.0) {
                fitting_map[node.id].emplace_back(dobor_50_45.get_name(), enumToStrng(dobor_50_45.get_type()));
                fitting_map[node.id].emplace_back(dobor_50_45.get_name(), enumToStrng(dobor_50_45.get_type()));
            }
            if (out_dia == 110.0) {
                fitting_map[node.id].emplace_back(dobor_110_45.get_name(), enumToStrng(dobor_110_45.get_type()));
            }

        } else {

            std::vector<CompositeFitting> filter_fit_out_in = filterByInOut(node);
            filterByAngle(filter_fit_out_in, node);
            if (!filter_fit_out_in.empty()) {
                node.cf = std::make_shared<CompositeFitting>(filter_fit_out_in.front());
                for (const auto &f: filter_fit_out_in.front().getAllFitings()) {
                    fitting_map[node.id].emplace_back(f->get_name(), enumToStrng(f->get_type()));
                }
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

//_________________________________________________________________________________________________
CompositeFitting::CompositeFitting() {
}

CompositeFitting::CompositeFitting(Fitting &main_fitting) {
    main_fitting_ = main_fitting;
    for (auto &c: main_fitting_.get_ins()) {
        child_fittings_[c];
    }
}

void CompositeFitting::operator=(const CompositeFitting &r) {
    main_fitting_ = r.main_fitting_;
    child_fittings_ = r.child_fittings_;
}

bool CompositeFitting::addChildFitting(Fitting &child_fitting, std::shared_ptr<Connector> connector) {
    child_fittings_[*connector].push_back(child_fitting);
    if (child_fitting.get_type() == FittingType::REDUCTION) {
        connector->reducDiametr(child_fitting.get_ins().front().get_diametr());
    } else if (child_fitting.get_type() == FittingType::TAP) {
        connector->rotate(child_fitting.get_ins().front().get_direction());
    }
}

bool CompositeFitting::is_composite() const { return true; }

std::vector<std::shared_ptr<Connector>> CompositeFitting::getIns() const {
    std::vector<std::shared_ptr<Connector>> res;
    for (auto it = child_fittings_.begin(); it != child_fittings_.end(); it++) {
        res.push_back(std::make_shared<Connector>(it->first));
    }
    Eigen::Quaternionf out_q = main_fitting_.get_out_direction();
    std::sort(res.begin(), res.end(), [&out_q](std::shared_ptr<Connector> &lhs, std::shared_ptr<Connector> &rhs) {
        return out_q.angularDistance(lhs->get_direction()) < out_q.angularDistance(rhs->get_direction());
    });
    return res;
}

std::vector<std::shared_ptr<Fitting>> CompositeFitting::getAllFitings() const {
    std::vector<std::shared_ptr<Fitting>> res;
    res.push_back(std::make_shared<Fitting>(main_fitting_));
    for (auto &[k, v]: child_fittings_) {
        for (auto &f: v) {
            res.push_back(std::make_shared<Fitting>(f));
        }
    }
    return res;
}

int CompositeFitting::getDegree() const {
    int res = 1;
    for (auto &[k, v]: child_fittings_) {
        for (auto &f: v) {
            res++;
        }
    }
    return res;
}

std::vector<Connector> CompositeFitting::getCompositeConnectors() const {
    std::vector<Connector> res;
    for (const auto &[main_con, childs]: child_fittings_) {
        Connector res_con = main_con;
        for (const auto &ch: childs) {
            if (ch.get_type() == REDUCTION) {
                res_con.reducDiametr(ch.get_ins().front().get_diametr());
            }
            if (ch.get_type() == TAP) {
                res_con.rotate(ch.get_ins().front().get_direction());
            }
        }
        res.push_back(res_con);
    }
    return res;
}

    std::optional<Connector> CompositeFitting::getConByQuat(Eigen::Quaternionf quat) {
        for (const auto &in: this->getCompositeConnectors()) {
            double step = 15;
            Eigen::Quaternionf null_quat(Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX()));
            double angle = radToDeg(null_quat.angularDistance(in.get_direction()));
            double check_angle = radToDeg(null_quat.angularDistance(quat));
            if (std::abs(angle-check_angle) < step) {
                return in;
            }
        }
        return std::optional<Connector>();
    }

    Connector CompositeFitting::getOut() const {
        return main_fitting_.get_out();
    }
//_________________________________________________________________________________________________
