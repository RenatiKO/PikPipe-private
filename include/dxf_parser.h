#ifndef DXF_PARSER_H
#define DXF_PARSER_H

//#include <dxflib/dl_creationadapter.h>
#include <dxfrw/drw_interface.h>
#include <vector>

#include <CGAL/Simple_cartesian.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
#include <unordered_map>

//typedef CGAL::Exact_predicates_inexact_constructions_kernel K ;
//typedef K::Point_2                    Point ;
//typedef CGAL::Polygon_2<K>            Polygon_2 ;
//typedef CGAL::Polygon_with_holes_2<K> Polygon_with_holes ;
//typedef CGAL::Straight_skeleton_2<K>  Ss ;
//typedef K::Line_2                     Line2d ;
//typedef K::Line_3                     Line ;


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
typedef Kernel::Segment_3 Segment3d;

using Point_2   = typename Kernel::Point_2;
using Segment_2 = typename Kernel::Segment_2;

typedef Kernel::Point_2                    Point ;
typedef CGAL::Polygon_2<Kernel>            Polygon_2 ;
typedef CGAL::Polygon_with_holes_2<Kernel> Polygon_with_holes ;
typedef CGAL::Straight_skeleton_2<Kernel>  Ss ;
typedef Kernel::Line_2                     Line2d ;
typedef Kernel::Line_3                     Line ;

//using Kernel    = CGAL::Simple_cartesian<double>;
//using Point_2   = typename Kernel::Point_2;
//using Segment_2 = typename Kernel::Segment_2;

enum SanType{NONE, TOILET, RAKOVINA, VANNA, SHOWER};

struct GraphNode{
    Point3d p;

    int number_of_target = -1;
    double diameter = -1;
    double weight_path = -1;
    double weight_eurist = 0;

        bool is_used = false;

    GraphNode* prev;
};

struct PipeNode {
    Point3d point;
    int id = -1;
    double diameter = -1;
    std::vector<PipeNode *> in;
    PipeNode *out = nullptr;
};

struct PipeToFront {
    Point3d point;
    int id = -1;
    double diameter = -1;
    Point3d next_p;
};

typedef std::vector<PipeToFront> PipesToFront;

struct PipeSegment {
    Segment3d pipe;
    double diameter;
};

typedef std::vector<PipeNode> PipeGraph;

class DxfParser : public DRW_Interface {
    std::string cur_layer_;
//    std::vector<SimpleLine> line_vec_;
    std::vector<Segment3d> line_vec_;
    std::vector<Segment3d> hole_vec_;
    std::vector<std::pair<Segment3d, Segment3d>> parallel_lines_;

    std::vector<Point> skelet_;

    std::vector<Segment3d> graph_edges_;

    std::unordered_map<Point3d, std::unordered_set<Point3d>> graph_;
    std::unordered_map<Point3d, std::vector<std::pair<double, Point3d>>> weighted_graph_;

    std::vector<std::pair<Point3d, SanType>> mission_targets;

    Point3d root_point_;

    std::vector<PipeNode> pipe_nodes_;

    bool isParallel(Segment3d l1, Segment3d l2);
    double linesDistance(SimpleLine l1, SimpleLine l2);

    void repairLines();
    void linesToPolygon();

    std::vector<PipeSegment> setPath(GraphNode* node);

    void addToPipePath(std::vector<PipeSegment> &pipes);

    void set_x(PipeNode* node);



public:
    DxfParser();
    PipesToFront getFrontNodes();
    std::vector<PipeNode> getPipeNodes();
//    std::vector<Segment3d> getPath();

    std::vector<PipeSegment> graphSearch();
    void weightGraph();

    void setRootOfGraph(Point3d v);

    Segment3d addVertexToGraph(Point3d v);
    void addMission(Point3d v, SanType t);

    std::vector<Point3d> getVertices();

    std::vector<Segment3d> getEdges();

    void calcNet();

    std::vector<Polygon_2> skeleton();

    std::vector<Polygon_2> wallSkeleton();

    Polygon_2 wallSkeletonWithHoles();

    std::vector<Segment3d> getLineVec();
    std::vector<Segment3d> getCenterLineVec();
    std::vector<Segment3d> getCenterLineVec1();

    void addHeader(const DRW_Header* data) override;

    /** Called for every line Type.  */
    void addLType(const DRW_LType &data) override {};

    /** Called for every layer. */
    void addLayer(const DRW_Layer &data) override {};

    /** Called for every dim style. */
    void addDimStyle(const DRW_Dimstyle &data) override {};

    /** Called for every VPORT table. */
    void addVport(const DRW_Vport &data) override {};

    /** Called for every text style. */
    void addTextStyle(const DRW_Textstyle &data) override {};

    /** Called for every AppId entry. */
    void addAppId(const DRW_AppId &data) override {};

    /**
     * Called for every block. Note: all entities added after this
     * command go into this block until endBlock() is called.
     *
     * @see endBlock()
     */
    void addBlock(const DRW_Block &data) override {};

    /**
     * In DWG called when the following entities corresponding to a
     * block different from the current. Note: all entities added after this
     * command go into this block until setBlock() is called already.
     *
     * int handle are the value of DRW_Block::handleBlock added with addBlock()
     */
    void setBlock(const int handle) override {};

    /** Called to end the current block */
    void endBlock() override {};

    /** Called for every point */
    void addPoint(const DRW_Point &data) override {};

    /** Called for every line */
    void addLine(const DRW_Line &data) override;

    /** Called for every ray */
    void addRay(const DRW_Ray &data) override {};

    /** Called for every xline */
    void addXline(const DRW_Xline &data) override {};

    /** Called for every arc */
    void addArc(const DRW_Arc &data) override {};

    /** Called for every circle */
    void addCircle(const DRW_Circle &data) override {};

    /** Called for every ellipse */
    void addEllipse(const DRW_Ellipse &data) override {};

    /** Called for every lwpolyline */
    void addLWPolyline(const DRW_LWPolyline &data) override {};

    /** Called for every polyline start */
    void addPolyline(const DRW_Polyline &data) override {};

    /** Called for every spline */
    void addSpline(const DRW_Spline *data) override {};

    /** Called for every spline knot value */
    void addKnot(const DRW_Entity &data) override {};

    /** Called for every insert. */
    void addInsert(const DRW_Insert &data) override {};

    /** Called for every trace start */
    void addTrace(const DRW_Trace &data) override {};

    /** Called for every 3dface start */
    void add3dFace(const DRW_3Dface &data) override {};

    /** Called for every solid start */
    void addSolid(const DRW_Solid &data) override {};


    /** Called for every Multi Text entity. */
    void addMText(const DRW_MText &data) override {};

    /** Called for every Text entity. */
    void addText(const DRW_Text &data) override {};

    /**
     * Called for every aligned dimension entity.
     */
    void addDimAlign(const DRW_DimAligned *data) override {};

    /**
     * Called for every linear or rotated dimension entity.
     */
    void addDimLinear(const DRW_DimLinear *data) override {};

    /**
     * Called for every radial dimension entity.
     */
    void addDimRadial(const DRW_DimRadial *data) override {};

    /**
     * Called for every diametric dimension entity.
     */
    void addDimDiametric(const DRW_DimDiametric *data) override {};

    /**
     * Called for every angular dimension (2 lines version) entity.
     */
    void addDimAngular(const DRW_DimAngular *data) override {};

    /**
     * Called for every angular dimension (3 points version) entity.
     */
    void addDimAngular3P(const DRW_DimAngular3p *data) override {};

    /**
     * Called for every ordinate dimension entity.
     */
    void addDimOrdinate(const DRW_DimOrdinate *data) override {};

    /**
     * Called for every leader start.
     */
    void addLeader(const DRW_Leader *data) override {};

    /**
     * Called for every hatch entity.
     */
    void addHatch(const DRW_Hatch *data) override {};

    /**
     * Called for every viewport entity.
     */
    void addViewport(const DRW_Viewport &data) override {};

    /**
     * Called for every image entity.
     */
    void addImage(const DRW_Image *data) override {};

    /**
     * Called for every image definition.
     */
    void linkImage(const DRW_ImageDef *data) override {};

    /**
     * Called for every comment in the DXF file (code 999).
     */
    void addComment(const char *comment) override {};

    void writeHeader(DRW_Header &data) override {};

    void writeBlocks() override {};

    void writeBlockRecords() override {};

    void writeEntities() override {};

    void writeLTypes() override {};

    void writeLayers() override {};

    void writeTextstyles() override {};

    void writeVports() override {};

    void writeDimstyles() override {};

    void writeAppId() override {};
};

#endif // DXF_PARSER_H
