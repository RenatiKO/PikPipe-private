#include "dxf_parser.h"
#include <iostream>
#include <cmath>
#include <limits>

//#define CGAL_USE_OSQP
#include <CGAL/create_offset_polygons_2.h>
//#include <CGAL/Shape_regularization/regularize_segments.h>
#include <CGAL/OSQP_quadratic_program_traits.h>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>

#include <fstream>


#include "header.h"

bool DxfParser::isParallel(Segment3d l1, Segment3d l2)
{
    //k = (y2 - y1)/(x2 - x1)
    //    double dx1 = l1.point(0).x() - l1.point(0).x();
    //    double dy1 = (l1[1].y - l1[0].y);

    //    double dx2 = l2[1].x - l2[0].x;
    //    double dy2 = (l2[1].y - l2[0].y);

    //    double k1 = dy1/dx1;
    //    double k2 = dy2/dx2;

    //    double diff = fabs(k1 - k2);

    //    return (diff < std::numeric_limits<double>::epsilon());
}

void DxfParser::repairLines()
{
    //    std::vector<Segment_2> segments;
    //    for (auto l : line_vec_) {
    //        segments.push_back({Point_2(l.point(0).x(), l.point(0).y()), Point_2(l.point(1).x(), l.point(1).y())});
    //    }
    ////    std::vector<Segment_2> segments = {
    ////        Segment_2(Point_2(0.2, 0.0), Point_2(1.2, 0.0)),
    ////        Segment_2(Point_2(1.2, 0.1), Point_2(2.2, 0.1)),
    ////        Segment_2(Point_2(2.2, 0.0), Point_2(2.0, 2.0)),
    ////        Segment_2(Point_2(2.0, 2.0), Point_2(1.0, 2.0)),
    ////        Segment_2(Point_2(1.0, 1.9), Point_2(0.0, 1.9)),
    ////        Segment_2(Point_2(0.0, 2.0), Point_2(0.2, 0.0))
    ////    };
    //    // Regularize all segments: both angles and offsets.
    //    CGAL::Shape_regularization::Segments::
    //        regularize_segments(segments);

    //    line_vec_.clear();
    //    for (auto it : segments) {
    //        std::cout << "(" << it.start().x() << ", " << it.start().y() << ") - (" << it.end().x() << ", " << it.end().y() << ")" << std::endl;
    //        line_vec_.push_back({{it.start().x(), it.start().y(), 0}, {it.end().x(), it.end().y(), 0}});
    //    }
}

void DxfParser::linesToPolygon()
{
    double minX = 10e6, minY = 10e6;
    for (auto it1 = line_vec_.begin(); it1 != line_vec_.end(); ++it1) {

    }
}

std::vector<PipeSegment> DxfParser::setPath(GraphNode *node)
{
    std::vector<PipeSegment> result;
    auto cur_node = node;
    while (cur_node->prev != nullptr) {
        cur_node->is_used = true;

//        double distance = sqrt(CGAL::squared_distance(cur_node->prev->p, cur_node->p));
//        double z_offset = distance * 0.03;

//        cur_node->prev->p += {0, 0, z_offset + cur_node->p.z()};

        result.push_back({{cur_node->prev->p, cur_node->p}, cur_node->prev->diameter});
        cur_node = cur_node->prev;
    }
    return result;
}

void DxfParser::addToPipePath(std::vector<PipeSegment> &pipes)
{
    pipe_nodes_.clear();
    std::map<Point3d, double> nodes;
    for (auto p : pipes) {
        nodes[p.pipe.start()] = std::max(p.diameter, nodes[p.pipe.start()]);
        nodes[p.pipe.end()] = std::max(p.diameter, nodes[p.pipe.end()]);
    }
    int id_cnt = 0;
    for (auto n : nodes) {
        pipe_nodes_.push_back(PipeNode());
        pipe_nodes_.back().point = n.first;
        pipe_nodes_.back().diameter = n.second;
        pipe_nodes_.back().id = id_cnt++;
    }

    for (auto p : pipes) {
        for (auto &pn : pipe_nodes_) {
            if (pn.point == p.pipe.start()) {
                //Добавление поинта как входа для другой ноды
                for (auto &nn : pipe_nodes_) {
                    if (nn.point == p.pipe.end()) {
                        nn.in.push_back(&pn);
                        nn.diameter = std::max(pn.diameter, nn.diameter);
                    }
                }
            }

            if (pn.point == p.pipe.end()) {
                //Добавление поинта как выхода для другой ноды
                for (auto &nn : pipe_nodes_) {
                    if (nn.point == p.pipe.start()) {
                        nn.out = (&pn);
                        //pn.diameter = std::max(pn.diameter, nn.diameter);
                    }
                }
            }
        }
    }

     for (auto &pn : pipe_nodes_) {
         if (pn.out == nullptr) {
             //проставляем высоты от стояка
             set_x(&pn);
         }
     }

     for (auto &pn : pipe_nodes_) {
         if (pn.in.empty()) {
             std::ofstream out("out.txt");
             PipeNode *n = &pn;
             while (n != nullptr) {
                 out << n->point.x() << " " << n->point.y() << " " << n->point.z() << std::endl;
                 n = n->out;
             }
             break;
         }
     }


     //    return result;
}

void DxfParser::set_x(PipeNode *node)
{
    if (node == nullptr) return;
    for (auto &in : node->in) {
        if (in->in.empty()) continue;

        double distance = sqrt(CGAL::squared_distance(node->point, in->point));
        double z_offset = distance * 0.03;

        in->point += {0, 0, z_offset + node->point.z()};

        set_x(in);
    }
}

PipesToFront DxfParser::getFrontNodes()
{
    PipesToFront ptf;
    PipeToFront p;

    for (auto n : pipe_nodes_) {
        p.diameter = n.diameter;
        p.id = n.id;
        p.point = n.point;
        if (n.out != nullptr) {
            p.next_p = n.out->point;
        }else {
            p.next_p = {0, 0, 0};
        }
        ptf.push_back(p);
    }
    return ptf;
}

std::vector<PipeNode> DxfParser::getPipeNodes()
{
    return pipe_nodes_;
}

DxfParser::DxfParser()
{
    //    repairLines();
    //    addVertexToGraph({0, 0, 0});
}

std::vector<PipeSegment> DxfParser::graphSearch()
{
    weightGraph();

    std::vector<PipeSegment> result;

    std::unordered_map<Point3d, GraphNode> nodes;
    for (auto g : graph_) {
        nodes.insert({g.first, GraphNode()});
        nodes[g.first].p = g.first;
        nodes[g.first].weight_path = -1;
        nodes[g.first].weight_eurist = sqrt(CGAL::squared_distance(g.first, root_point_));
        nodes[g.first].prev = nullptr;
    }

    struct customLess
    {
        bool operator()(const GraphNode* l, const GraphNode* r) const { return (l->weight_eurist + l->weight_path) > (r->weight_eurist + r->weight_path); }
    } ;

//    auto cur_start = mission_targets[0].first;
    for (int num_targ = 0; num_targ < mission_targets.size(); ++num_targ) {
        auto cur_start = &nodes[mission_targets[num_targ].first];
        cur_start->weight_path = 0;
        cur_start->number_of_target = num_targ;
        cur_start->diameter = (mission_targets[num_targ].second == TOILET) ? 110 : 50;
        cur_start->prev = nullptr;

        //BFS
        //    std::priority_queue<std::pair<double, Point3d>> q;
        std::priority_queue<GraphNode*, std::vector<GraphNode*>, customLess> q;
        q.push(cur_start);

        while (!q.empty()) {
            auto v = q.top();
            q.pop();

    //        cur_start = v;

            if (v->p == root_point_ || v->is_used == true/*(v->number_of_target < num_targ && v->number_of_target >= 0)*/) {
                //set result
                auto p = setPath(v);

                result.insert(result.end(), p.begin(), p.end());
                break;

//                return setPath(v);
            }
            auto list = weighted_graph_[v->p];
            for (auto to : weighted_graph_[v->p]) {
                double to_w = v->weight_path + sqrt(CGAL::squared_distance(v->p, to.second));//to.first;
                auto to_node = &nodes[to.second];
                //            int to = g[v][i];
                if (to_node->number_of_target != num_targ || to_w < to_node->weight_path) {
                    to_node->weight_path = to_w;
                    to_node->prev = v;
                    to_node->number_of_target = num_targ;
                    to_node->diameter = to_node->is_used ? std::max(to_node->diameter, v->diameter) : v->diameter;
                    q.push (to_node);
                }
            }
        }
    }
    addToPipePath(result);
    return result;
}

void DxfParser::weightGraph()
{
    weighted_graph_.clear();

    for (auto vs : graph_) {
        for (auto ve : vs.second) {
            weighted_graph_[vs.first].push_back({sqrt(CGAL::squared_distance(ve, root_point_)), ve});
        }
        std::sort(weighted_graph_[vs.first].begin(), weighted_graph_[vs.first].end());
    }
}

void DxfParser::setRootOfGraph(Point3d v)
{
    root_point_ = v;
}

Segment3d DxfParser::addVertexToGraph(Point3d v)
{
    Segment3d nearest_seg;
    double min_dist = 10e8;

    //    std::cout << "sq_dist = " << CGAL::squared_distanceC3(1, 1, 0, 3, 1, 0) << std::endl;

    for (auto vs : graph_) {
        for (auto ve : vs.second) {
            if (auto dist = CGAL::squared_distance(Segment3d{vs.first, ve}, v); min_dist > dist) {
                min_dist = dist;
                nearest_seg = {vs.first, ve};
            }
        }
    }
    Line line(nearest_seg);
    Point3d new_vert = line.projection(v);

    std::cout << "nearest == " << nearest_seg.start().x() << ":" << nearest_seg.start().y() << "\\//" << nearest_seg.end().x() << ":" << nearest_seg.end().y() << std::endl;

    graph_[nearest_seg.start()].erase(nearest_seg.end());
    graph_[nearest_seg.end()].erase(nearest_seg.start());

    graph_[nearest_seg.start()].insert(new_vert);
    graph_[nearest_seg.end()].insert(new_vert);

    graph_[v].insert(new_vert);

    graph_[new_vert].insert(v);
    graph_[new_vert].insert(nearest_seg.start());
    graph_[new_vert].insert(nearest_seg.end());

    return nearest_seg;
}

void DxfParser::addMission(Point3d v, SanType t)
{
    mission_targets.push_back({v, t});
}

std::vector<Point3d> DxfParser::getVertices()
{
    std::vector<Point3d> verts;

    for (auto v : graph_) {
        verts.push_back(v.first);
    }

    return verts;
}

std::vector<Segment3d> DxfParser::getEdges()
{
    std::vector<Segment3d> edges;

    auto ifExist = [](std::vector<Segment3d>& v, Segment3d s) -> bool {
        for (auto seg : v) {
            if (seg == s || seg.opposite() == s) {
                return true;
            }
        }
        return false;
    };

    for (auto v : graph_) {
        for (auto v2 : v.second) {
            if(!ifExist(edges, {v.first, v2})) {
                edges.push_back({v.first, v2});
            }
        }
    }

    return edges;
}

void DxfParser::calcNet()
{
    //    namespace bg = boost::geometry;
    //    for (auto i1 = 0; i1 < line_vec_.size(); ) {
    //        if (line_vec_.size() <= 1) return;
    //        double min_dist = 10e8;
    //        auto nearest = i1+1;
    //        for (auto i2 = i1+1; i2 < line_vec_.size(); ++i2) {
    //            if (isParallel(line_vec_[i1], line_vec_[i2])) {
    //                if (auto d = bg::distance(line_vec_[i1], line_vec_[i2]); d < min_dist) {
    //                    min_dist = bg::distance(line_vec_[i1], line_vec_[i2]);
    //                    nearest = i2;
    //                }
    //            }
    //        }

    //        if (line_vec_.empty()) return;

    //        parallel_lines_.push_back({line_vec_[i1], line_vec_[nearest]});
    //        line_vec_.erase(line_vec_.begin()+nearest);
    //        line_vec_.erase(line_vec_.begin() + i1);

    ////        i1;
    //    }
}

std::vector<Polygon_2> DxfParser::skeleton()
{
    Polygon_2 res1, res2;
    Polygon_2 poly ;
    //    poly.push_back( Point(0, 0) ) ;
    //    poly.push_back( Point(0, 6) ) ;
    //    poly.push_back( Point(6, 6) ) ;
    //    poly.push_back( Point(6, 0) ) ;
    //    poly.push_back( Point(4, 0) ) ;
    //    poly.push_back( Point(4, 4) ) ;
    //    poly.push_back( Point(2, 4) ) ;
    //    poly.push_back( Point(2, 0) ) ;

    poly.push_back( Point(101, 100) ) ;
    poly.push_back( Point(300, 100) ) ;
    poly.push_back( Point(300, 150) ) ;
    poly.push_back( Point(150, 150) ) ;
    poly.push_back( Point(150, 250) ) ;
    poly.push_back( Point(300, 250) ) ;
    poly.push_back( Point(300, 300) ) ;
    poly.push_back( Point(101, 300) ) ;

    //    poly.push_back( Point(0, 30) ) ;
    //    poly.push_back( Point(0, 0) ) ;

    assert(poly.is_counterclockwise_oriented());
    // You can pass the polygon via an iterator pair
    auto iss = CGAL::create_interior_straight_skeleton_2(poly.vertices_begin(), poly.vertices_end());
    // Or you can pass the polygon directly, as below.
    // To create an exterior straight skeleton you need to specify a maximum offset.
    double lMaxOffset = 100 ;
    auto oss = CGAL::create_exterior_straight_skeleton_2(lMaxOffset, poly);

    for (auto it = iss->halfedges_begin(); it != iss->halfedges_end(); ++it) {
        if(it->is_inner_bisector()) {
            res1.push_back({it->vertex()->point().x(), it->vertex()->point().y()});
            res1.push_back({it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y()});
        }
    }

    for (auto it = oss->halfedges_begin(); it != oss->halfedges_end(); ++it) {
        res2.push_back({it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y()});
    }

    print_straight_skeleton(*iss);
    print_straight_skeleton(*oss);

    return {poly, res1, res2};
    //    print_straight_skeleton(*iss);
}

std::vector<Polygon_2> DxfParser::wallSkeleton()
{
    Polygon_2 res1, res2;
    Polygon_2 poly ;

    //    for (auto rit = line_vec_.rbegin(); rit != line_vec_.rend(); ++rit) {
    //        poly.push_back(Point(rit->point(1).x(), rit->point(1).y()));
    //    }

    //    poly.push_back(Point(line_vec_.front().point(0).x(), line_vec_.front().point(0).y()));

    for (auto it = line_vec_.begin(); it != line_vec_.end(); ++it) {
        poly.push_back(Point(it->point(0).x(), it->point(0).y()));
    }

    //    poly.push_back(Point(line_vec_.back().point(1).x(), line_vec_.back().point(1).y()));
    //    poly.push_back(Point(line_vec_.front().point(0).x(), line_vec_.front().point(0).y()));

    assert(poly.is_counterclockwise_oriented());
    // You can pass the polygon via an iterator pair


    //    auto iss = CGAL::create_interior_skeleton_and_offset_polygons_2(30., poly/*poly.vertices_begin(), poly.vertices_end()*/);

    auto iss = CGAL::create_interior_straight_skeleton_2(poly.vertices_begin(), poly.vertices_end());

    // Or you can pass the polygon directly, as below.
    // To create an exterior straight skeleton you need to specify a maximum offset.
    double lMaxOffset = 100 ;
    auto oss = CGAL::create_exterior_straight_skeleton_2(lMaxOffset, poly);

    for (auto it = iss->halfedges_begin(); it != iss->halfedges_end(); ++it) {
        if(it->is_inner_bisector()) {
            res1.push_back({it->vertex()->point().x(), it->vertex()->point().y()});
            res1.push_back({it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y()});

            graph_[{it->vertex()->point().x(), it->vertex()->point().y(), 0}].insert({it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y(), 0});
        }
    }

    //    for (auto it = iss[0]->vertices_begin(); it != iss[0]->vertices_end(); ++it) {
    ////            if(it->is_inner_bisector())
    //        res1.push_back({it->x(), it->y()});
    //        }

    for (auto it = oss->halfedges_begin(); it != oss->halfedges_end(); ++it) {
        res2.push_back({it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y()});
    }

    //    print_polygons(iss);
    print_straight_skeleton(*iss);
    print_straight_skeleton(*oss);

    return {poly, res1, res2};
}

Polygon_2 DxfParser::wallSkeletonWithHoles()
{
    if (hole_vec_.empty()) {
        auto r = wallSkeleton();
        return r[1];
    }
    Polygon_2 outer, res ;
    for (auto it = line_vec_.begin(); it != line_vec_.end(); ++it) {
        outer.push_back(Point(it->point(0).x(), it->point(0).y()));
    }
    Polygon_2 hole ;
    for (auto it = hole_vec_.begin(); it != hole_vec_.end(); ++it) {
        hole.push_back(Point(it->point(0).x(), it->point(0).y()));
    }
    assert(outer.is_counterclockwise_oriented());
    assert(hole.is_clockwise_oriented());
    Polygon_with_holes poly( outer ) ;
    poly.add_hole( hole ) ;
    auto iss = CGAL::create_interior_straight_skeleton_2(poly);
    print_straight_skeleton(*iss);

    for (auto it = iss->halfedges_begin(); it != iss->halfedges_end(); ++it) {
        if(it->is_inner_bisector()) {
            res.push_back({it->vertex()->point().x(), it->vertex()->point().y()});
            res.push_back({it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y()});

            graph_[{it->vertex()->point().x(), it->vertex()->point().y(), 0}].insert({it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y(), 0});

            //            graph_edges_.push_back({{it->vertex()->point().x(), it->vertex()->point().y(), 0},
            //                                       {it->opposite()->vertex()->point().x(), it->opposite()->vertex()->point().y(), 0}});
        }
    }

    return res;
}

std::vector<Segment3d> DxfParser::getLineVec()
{
    //    repairLines();
    return line_vec_;
}

std::vector<Segment3d> DxfParser::getCenterLineVec()
{
    //    std::vector<LineString2d> res;

    //    for (auto ll : parallel_lines_) {
    //        res.push_back(ll.first);
    //    }

    //    return res;
}

std::vector<Segment3d> DxfParser::getCenterLineVec1()
{
    //    std::vector<LineString2d> res;

    //    for (auto ll : parallel_lines_) {
    //        res.push_back(ll.second);
    //    }

    //    return res;
}

void DxfParser::addHeader(const DRW_Header *data)
{
    for (const auto &h : data->vars) {
        std::cout << h.first << " - " << h.second->type() << std::endl;
        if (h.first == "$PEXTMIN") {
            std::cout << "X == " << h.second->content.v->x << std::endl;
            std::cout << "Y == " << h.second->content.v->y << std::endl;
        }
        if (h.first == "$PEXTMAX") {
            std::cout << "X == " << h.second->content.v->x << std::endl;
            std::cout << "Y == " << h.second->content.v->y << std::endl;
        }
    }


}

void DxfParser::addLine(const DRW_Line &data)
{
    if (data.layer == /*"I-WALL-4"*/"MY_WALL") {
        std::cout << "Line: " << data.basePoint.x << "/" << data.basePoint.y
                  << " " << data.secPoint.x << "/" << data.secPoint.y << std::endl;

        line_vec_.push_back( {{data.basePoint.x, data.basePoint.y, data.basePoint.z},
                              {data.secPoint.x, data.secPoint.y, data.secPoint.z}} );
    }

    if (data.layer == /*"I-WALL-4"*/"MY_HOLE") {
        hole_vec_.push_back( {{data.basePoint.x, data.basePoint.y, data.basePoint.z},
                              {data.secPoint.x, data.secPoint.y, data.secPoint.z}} );
    }
}
