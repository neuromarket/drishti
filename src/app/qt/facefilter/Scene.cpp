/*!
  @file   finder/Scene.cpp
  @author David Hirvonen
  @brief  Scene viewed by the camera represented by low level primitives: (corners, face, flow, etc.)

  \copyright Copyright 2014-2016 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}

*/

#include "Scene.hpp"

static float getAngle(const cv::Point2f &p)
{
    return (std::atan2(p.y, p.x) + M_PI) * 180.0 / M_PI;
}

void flowToDrawings(const std::vector<cv::Vec4f> &flow, LineDrawingVec &drawings, const cv::Mat3f &colorMap)
{
    drawings.reserve(drawings.size() + flow.size());
    for(int i = 0; i < flow.size(); i++)
    {
        ogles_gpgpu::LineDrawing drawing;
        const auto &f = flow[i];
        const cv::Point2f p(f[0], f[1]);
        const cv::Point2f q(f[2], f[3]);
        const float angle = getAngle(q);
        const int index = std::max(0, std::min(int(angle+0.5f), int(colorMap.cols-1)));

        drawing.strip = false; // STRIP == FALSE
        drawing.color = colorMap(index);
        drawing.contours = {{ p, p + q }};
        drawings.emplace_back(drawing);
    }
}

void pointsToCrosses(const std::vector<cv::Point2f> &points, LineDrawingVec &crosses)
{
    const float span = 8.0;
    cv::Point2f dx(span, 0.0), dy(0.0, span);

    for(const auto &p : points)
    {
        ogles_gpgpu::LineDrawing cross;
        cross.strip = false; // STRIP == FALSE
        cross.contours = {{ p - dx, p + dx }, { p - dy, p + dy }};
        cross.index = { 0 };
        crosses.emplace_back(cross);
    }
}

void rectangleToDrawing(const cv::Rect &r, ogles_gpgpu::LineDrawing &drawing, bool closed)
{
    const cv::Point2f tl = r.tl(), br = r.br(), tr(br.x, tl.y), bl(tl.x, br.y);
    drawing.strip = true; // STRIP == TRUE
    drawing.contours = {{ tl, tr, br, bl }};
    if(closed)
    {
        drawing.contours.back().push_back(tl);
    }
    drawing.index = { 0 };
}

void rectanglesToDrawings(const std::vector<cv::Rect> &rectangles, LineDrawingVec &drawings)
{
    for(const auto &r : rectangles)
    {
        ogles_gpgpu::LineDrawing drawing;
        rectangleToDrawing(r, drawing);
        drawings.emplace_back(drawing);
    }
}

void faceToDrawing(const drishti::face::FaceModel &face, ogles_gpgpu::LineDrawing &drawing)
{
    auto parts = face.getFaceParts(true);
    drawing.strip = true; // STRIP == TRUE
    drawing.roi = face.roi;
    for(int i = 0; i < parts.size(); i++)
    {
        for(auto &c : parts[i])
        {
            if(c.size())
            {
                drawing.index.push_back(i);
                drawing.contours.push_back(c);
            }
        }
    }
}

void facesToDrawings(const std::vector<drishti::face::FaceModel> &faces, LineDrawingVec &drawings)
{
    for(const auto &f : faces)
    {
        {
            ogles_gpgpu::LineDrawing drawing;
            rectangleToDrawing(f.roi, drawing);
            drawings.emplace_back(drawing);
        }
        {
            ogles_gpgpu::LineDrawing drawing;
            faceToDrawing(f, drawing);
            drawings.emplace_back(drawing);
        }
    }
}