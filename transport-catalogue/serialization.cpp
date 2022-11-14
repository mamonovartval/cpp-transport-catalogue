#include "serialization.h"

#include"svg.h"
#include <string>

transport_catalogue_serialize::TC serialization::CreateTC(const tc::TransportCatalogue& catalogue)
{
    transport_catalogue_serialize::TC tc;

    int i = 0;
    // read Stops data from tc
    for (const auto& stopData : catalogue.GetStops()) {
        // create stop object
        transport_catalogue_serialize::Stop stop;
        stop.set_namestop(stopData.nameStop);
        stop.set_latitude(stopData.latitude);
        stop.set_longitude(stopData.longitude);
        // make setup stop to db
        *tc.mutable_stops()->Add() = std::move(stop);
        ++i;
    }

    // read Buses data from tc
    for (const auto& [name, busData] : catalogue.GetSortedBuses()) {
        // create bus object
        transport_catalogue_serialize::Bus bus;
        bus.set_isring(busData->isRing);
        bus.set_namebus(busData->nameBus);
       
        for (int j = 0; j < busData->ptr_ToStops.size(); ++j) {
            // looking for index for each stop
            std::optional<int> idStop = std::nullopt;
            for (int k = 0; k < tc.stops().size(); ++k) {
                if (busData->ptr_ToStops[j]->nameStop == tc.stops().operator[](k).namestop()) {
                    idStop = k;
                    break;
                }
            }
            if (idStop.has_value()) {
                *bus.mutable_ptrtostops()->Add() = idStop.value();
            }
        }
        // make setup bus to db
        *tc.mutable_buses()->Add() = std::move(bus);
    }

    // read Distances between stops
    for (const auto& [stop, distance] : catalogue.GetAllDistances()) {
        transport_catalogue_serialize::DistanceBetweenStop dist;
        // getting stop from
        dist.set_firststop(stop.first->nameStop);
        // declare stop to
        dist.set_secondstop(stop.second->nameStop);
        // getting distance between stops
        dist.set_distance(distance);
        // make setup dist to db
        *tc.mutable_distbtwnstop()->Add() = std::move(dist);
    }
    return tc;
}

render_settings_serialize::RenderSet serialization::CreateRenderer(const renderer::Settings& settings)
{
    // read Render settings from tc
    render_settings_serialize::RenderSet renderSet;
   renderSet.set_width(settings.width);
   renderSet.set_height(settings.height);
   renderSet.set_padding(settings.padding);
   renderSet.set_linewidth(settings.line_width);
   renderSet.set_stopradius(settings.stop_radius);
   renderSet.set_bus_labelfontsize(settings.bus_label_font_size);
   for (const auto& blo : settings.bus_label_offset) {
       renderSet.add_buslabeloffset(blo);
   }
   renderSet.set_stoplabelfontsize(settings.stop_label_font_size);
   for (const auto& slo : settings.stop_label_offset) {
       renderSet.add_stoplabeloffset(slo);
   }
   // getting data of Color for underlayer color
    svg_serialize::Color color;
    if (std::holds_alternative<svg::Rgb>(settings.underlayer_color)) {
        svg_serialize::Rgb rgb_color;
        svg::Rgb rgb = std::get<svg::Rgb>(settings.underlayer_color);
        rgb_color.set_red(rgb.red);
        rgb_color.set_green(rgb.green);
        rgb_color.set_blue(rgb.blue);
   
        *color.mutable_rgbcolor() = std::move(rgb_color);
    }
    else if (std::holds_alternative<svg::Rgba>(settings.underlayer_color)) {
        svg_serialize::Rgba rgba_color;
        svg::Rgba rgba = std::get<svg::Rgba>(settings.underlayer_color);
        rgba_color.set_red(rgba.red);
        rgba_color.set_green(rgba.green);
        rgba_color.set_blue(rgba.blue);
        rgba_color.set_opacity(rgba.opacity);
        *color.mutable_rgbacolor() = std::move(rgba_color);
    }
    else if (std::holds_alternative<std::string>(settings.underlayer_color)) {
        std::string str_color = std::get<std::string>(settings.underlayer_color);
        color.set_scolor(std::move(str_color));
    }
    *renderSet.mutable_underlayercolor() = std::move(color);
    // end add underlayer color
   
    renderSet.set_underlayerwidth(settings.underlayer_width);
   
    // getting data of Color for color palette
    for (const auto& cp : settings.color_palette) {
        svg_serialize::Color color;
        if (std::holds_alternative<svg::Rgb>(cp)) {
            svg_serialize::Rgb rgb_color;
            svg::Rgb rgb = std::get<svg::Rgb>(cp);
            rgb_color.set_red(rgb.red);
            rgb_color.set_green(rgb.green);
            rgb_color.set_blue(rgb.blue);
   
            *color.mutable_rgbcolor() = std::move(rgb_color);
        }
        else if (std::holds_alternative<svg::Rgba>(cp)) {
            svg_serialize::Rgba rgba_color;
            svg::Rgba rgba = std::get<svg::Rgba>(cp);
            rgba_color.set_red(rgba.red);
            rgba_color.set_green(rgba.green);
            rgba_color.set_blue(rgba.blue);
            rgba_color.set_opacity(rgba.opacity);
   
            *color.mutable_rgbacolor() = std::move(rgba_color);
        }
        else if (std::holds_alternative<std::string>(cp)) {
            std::string str_color = std::get<std::string>(cp);
            color.set_scolor(std::move(str_color));
        }
        *renderSet.mutable_colorpalette()->Add() = std::move(color);
        // end add color palette
    }
   
    return renderSet;
}

router_serialize::Router serialization::CreateRouter(const graph::TransportRouter& rdb,
    const int waitTime, const double velocity)
{
    router_serialize::Router rt;
    // read route settings
    router_serialize::RouterSettings routeSet;
    routeSet.set_buswaittime(waitTime);
    routeSet.set_busvelocity(velocity);

    // make setup route settings to db
    *rt.mutable_routersettings() = std::move(routeSet);
   
    // read graph data
    const graph::DirectedWeightedGraph<double>& g = rdb.GetMakedGraph().GetGraph();
    graph_serialize::Graph graph_db;
   
    size_t vertex_count = g.GetVertexCount();
    size_t edge_count = g.GetEdgeCount();
   
    // add edge data to db
    for (size_t i = 0; i < edge_count; ++i) {
        const graph::Edge<double>& edge = g.GetEdge(i);
        graph_serialize::Edge edge_db;
        edge_db.set_edgename(edge.name);
        edge_db.set_spancount(edge.span_count);
        edge_db.set_from(edge.from);
        edge_db.set_to(edge.to);
        edge_db.set_weight(edge.weight);
        *graph_db.add_edge() = std::move(edge_db);
    }
    // add vertex data to db
    for (size_t i = 0; i < vertex_count; ++i) {
        graph_serialize::Vertex vertex_db;
        for (const auto& edge_id : g.GetIncidentEdges(i)) {
            vertex_db.add_edgeid(edge_id);
        }
        *graph_db.add_vertex() = std::move(vertex_db);
    }
   
    // make setup graph to db
    *rt.mutable_graph() = graph_db;
   
    // read stop ids
    for (const auto& [name, id] : rdb.GetMakedGraph().GetStopIds()) {
        router_serialize::StopId s_id;
        s_id.set_stopidname(name);
        s_id.set_id(id);
        *rt.add_stopid() = std::move(s_id);
    }

    return rt;
}

void serialization::SerilalizeData(const std::filesystem::path& path, 
    const transport_catalogue_serialize::TC& obj_catalogue, 
    const render_settings_serialize::RenderSet& obj_rendSet,
    const router_serialize::Router& obj_router)
{
    std::ofstream out_file(path, std::ios::binary | std::ios::trunc);
    transport_catalogue_serialize::TCFull full_tc;
    *full_tc.mutable_tc_catalog() = obj_catalogue;
    *full_tc.mutable_render() = obj_rendSet;
    *full_tc.mutable_router() = obj_router;

    full_tc.SerializeToOstream(&out_file);
}

std::optional<transport_catalogue_serialize::TCFull> serialization::Deserelization(const std::filesystem::path &path) {

    std::fstream in_file(path, std::ios::in | std::ios::binary);
    transport_catalogue_serialize::TCFull tc_full;

    if (!tc_full.ParseFromIstream(&in_file)) {
        return std::nullopt;
    }

    return tc_full;
}

std::optional<transport_catalogue_serialize::TC> serialization::DeserelizationTC(std::optional<transport_catalogue_serialize::TCFull> &tc_full) {
    return tc_full->tc_catalog();
}

std::optional<render_settings_serialize::RenderSet> serialization::DeserelizationRenderer(std::optional<transport_catalogue_serialize::TCFull> &tc_full) {
    return tc_full->render();
}

std::optional<router_serialize::Router> serialization::DeserelizationRouter(std::optional<transport_catalogue_serialize::TCFull> &tc_full) {
    return tc_full->router();
}

void serialization::InitiliaziationTransportCatalogue(const std::optional<transport_catalogue_serialize::TC>& db,
    tc::TransportCatalogue& catalogue)
{
    // add stop to catalogue
    for (int i = 0; i < db.value().stops().size(); ++i) {
        // getting data for each stop from db
        const transport_catalogue_serialize::Stop& s_data = db.value().stops().operator[](i);
        // put stop data to catalogue
        catalogue.AddStopToBase(s_data.namestop(), s_data.latitude(), s_data.longitude());
    }

    // add distances between stops to catalogue
    for (int i = 0; i < db.value().distbtwnstop().size(); ++i) {
        // getting data for each distance
        const transport_catalogue_serialize::DistanceBetweenStop& d_data = db.value().distbtwnstop().operator[](i);
        // getting ptr for first stop from catalogue
        const domain::Stop* ptr_from = catalogue.SearchStop(d_data.firststop());
        // getting ptr for second stop from catalogue
        const domain::Stop* ptr_to = catalogue.SearchStop(d_data.secondstop());
        // set distance to catalogue
        catalogue.SetDistanceBetweenStops(std::make_pair(ptr_from, ptr_to), d_data.distance());
    }

    // add route data to catalogue
    for (int i = 0; i < db.value().buses().size(); ++i) {
        // getting data for each bus
        const transport_catalogue_serialize::Bus b_data = db.value().buses().operator[](i);
        // declare list of stops on route
        std::vector<std::string> routeStops;
        routeStops.reserve(b_data.ptrtostops().size());
        // getting ptr to stops
        auto ptr_stops = b_data.ptrtostops();
        // put stops data to container
        for (int j = 0; j < ptr_stops.size(); ++j) {
            // get index stop
            int idStop = ptr_stops.operator[](j);
            // get stop data from db
            transport_catalogue_serialize::Stop s_data = db.value().stops().operator[](idStop);
            // put stop to container
            routeStops.push_back(s_data.namestop());
        }
        if (routeStops.empty()) {
            continue;
        }
        // put bus data to catalogue
        catalogue.AddBus(db.value().buses().at(i).namebus(),
            routeStops, b_data.isring());
    }
}

void serialization::InitializationRenderSettings(const std::optional<render_settings_serialize::RenderSet>& sets_db,
    renderer::Settings& settings)
{
    // getting settings from db
    settings.width = sets_db.value().width();
    settings.height = sets_db.value().height();
    settings.padding = sets_db.value().padding();
    settings.line_width = sets_db.value().linewidth();
    settings.stop_radius = sets_db.value().stopradius();
    settings.bus_label_font_size = sets_db.value().bus_labelfontsize();
    for (int s = 0; s < sets_db.value().buslabeloffset().size(); ++s) {
        settings.bus_label_offset.push_back(sets_db.value().buslabeloffset().operator[](s));
    }
    settings.stop_label_font_size = sets_db.value().stoplabelfontsize();
    for (int s = 0; s < sets_db.value().stoplabeloffset().size(); ++s) {
        settings.stop_label_offset.push_back(sets_db.value().stoplabeloffset().operator[](s));
    }
    // getting color from db
    const svg_serialize::Color& c_data = sets_db.value().underlayercolor();
    SetColorSetting(c_data, settings);

    settings.underlayer_width = sets_db.value().underlayerwidth();
    for (int s = 0; s < sets_db.value().colorpalette().size(); ++s) {
        // getting color from db
        const svg_serialize::Color& c_data = sets_db.value().colorpalette().operator[](s);
        SetColorSetting(c_data, settings);
    }
}

void serialization::InitializationRouter(const std::optional<router_serialize::Router>& router_db,
    graph::TransportGraph& tr)
{
    double velocity = router_db.value().routersettings().busvelocity();
    int bus_wait_time = router_db.value().routersettings().buswaittime();

    // getting graph data from db
    graph_serialize::Graph g_data = router_db.value().graph();
    // init vector of Edge for init graph
    std::vector<graph::Edge<double>> edges(g_data.edge_size());
    // init container for incidence list for init graph
    std::vector<std::vector<graph::EdgeId>> incidence_lists(g_data.vertex_size());
    
    for (size_t i = 0; i < edges.size(); ++i) {
        const graph_serialize::Edge& e_data = g_data.edge(i);
        // init edge by value from db
        edges[i] = {e_data.edgename(), static_cast<size_t>(e_data.spancount()),
        static_cast<size_t>(e_data.from()), static_cast<size_t>(e_data.to()), e_data.weight()};
    }
    for (size_t i = 0; i < incidence_lists.size(); ++i) {
        const graph_serialize::Vertex& v_data = g_data.vertex(i);
        incidence_lists[i].reserve(v_data.edgeid().size());
        for (const auto& id : v_data.edgeid()) {
            incidence_lists[i].push_back(id);
        }
    }
    // init graph
    graph::DirectedWeightedGraph graph_db(edges, incidence_lists);
    // set graph to transport router
    tr.SetGraph(std::move(graph_db));

    // get stop ids from db
    graph::StopNameToVertexId stop_ids;
    for (const auto& s : router_db.value().stopid()) {
        stop_ids[s.stopidname()] = s.id();
    }
    // set stop ids to transport router
    tr.SetStopIds(std::move(stop_ids));
}

void serialization::SetColorSetting(const svg_serialize::Color& c_data, renderer::Settings& settings)
{
    if (c_data.has_rgbcolor()) {
        svg::Rgb rgb_color;
        rgb_color.red = c_data.rgbcolor().red();
        rgb_color.green = c_data.rgbcolor().green();
        rgb_color.blue = c_data.rgbcolor().blue();
        settings.color_palette.push_back(rgb_color);
    }
    else if (c_data.has_rgbacolor()) {
        svg::Rgba rgba_color;
        rgba_color.red = c_data.rgbacolor().red();
        rgba_color.green = c_data.rgbacolor().green();
        rgba_color.blue = c_data.rgbacolor().blue();
        rgba_color.opacity = c_data.rgbacolor().opacity();
        settings.color_palette.push_back(rgba_color);
    }
    else {
        settings.color_palette.push_back(c_data.scolor());
    }
}

