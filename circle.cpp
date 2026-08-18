
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")


#include <uxtheme.h>
#pragma comment(lib,"uxtheme")

#include "include/window.h"
#include "include/sizedStruct.h"

#include <ranges>
#include <print>
#include <algorithm>
#include <vector>
#include <numeric>

class CircleWindow : public Window
{
using Point = std::pair<int,int>;

public:
    CircleWindow() : Window("Circle"),
                     blackPen{ CreatePen(PS_SOLID,1,RGB(0,0,0)) },
                     redPen  { CreatePen(PS_SOLID,1,RGB(255,0,0)) }
    {
    }


private:

    HPEN                    blackPen;
    HPEN                    redPen;

    bool                    drawing{};

    std::vector<Point>      points;

    bool                    done{};

    Point                   center;
    double                  radius;
    double                  circularity;
    

    void lButtonDown()
    {
        points.clear();
        points.reserve(1000);

        drawing=true;
        done=false;
        InvalidateRect(window,nullptr,true);
    }

    void lButtonUp()
    {
        drawing=false;

        if(points.empty())
        {
            return;
        }

        auto left   = std::ranges::min_element(points, {}, &Point::first)->first;
        auto right  = std::ranges::max_element(points, {}, &Point::first)->first;
        auto top    = std::ranges::min_element(points, {}, &Point::second)->second;
        auto bottom = std::ranges::max_element(points, {}, &Point::second)->second;


        center.first  = (right+left)/2;
        center.second = (bottom+top)/2;

        auto lengths = std::vector<double>{};

        for(auto const &point : points)
        {
            lengths.push_back(std::hypot(point.first  - center.first,point.second - center.second));  
        }


        radius = std::ranges::fold_left(lengths ,  0.0, std::plus<>()) / lengths.size();

      

        auto sq_sum = std::ranges::fold_left(lengths, 0.0, [&](double acc, double val) {
            double diff = val - radius;
            return acc + (diff * diff);
        });

        auto variance = sq_sum / (lengths.size()-1);
        circularity = 100.0 -  (100.0*sqrt(variance)/ radius);


        done=true;

        InvalidateRect(window,nullptr,false);
    }


    void mouseMove(LPARAM l)
    {
        if(!drawing)
        {
            return;
        }

        auto x = LOWORD(l);         
        auto y = HIWORD(l);

        points.emplace_back(x,y);

        InvalidateRect(window,nullptr,false);
    }



    void SetupWindow()
    {
        Window::center();
    }

    bool proc(UINT m, WPARAM w, LPARAM l) override
    {
        switch(m)
        {
        case WM_WINDOW_CREATED:
            SetupWindow();
            return true;

        case WM_LBUTTONDOWN:
            lButtonDown();
            return true;

        case WM_LBUTTONUP:
            lButtonUp();
            return true;

        case WM_MOUSEMOVE:
            mouseMove(l);
            return true;
        }

        return false;
    }



    void paint(PAINTSTRUCT const &, HDC windowDc)  override
    {
        auto nullBrush = GetStockObject(NULL_BRUSH);

        auto oldPen    = SelectObject(windowDc,blackPen);
        auto oldBrush  = SelectObject(windowDc,nullBrush);

        for(auto const &point : points)
        {
            Ellipse(windowDc, 
                    point.first-1,    
                    point.second-1,
                    point.first+1,
                    point.second+1);
        }

        if(done)
        {
            SelectObject(windowDc,redPen);

            Ellipse(windowDc, 
                    center.first-2,    
                    center.second-2,
                    center.first+2,
                    center.second+2);

            Ellipse(windowDc, 
                    center.first-radius,    
                    center.second-radius,
                    center.first+radius,
                    center.second+radius);


            auto text = std::format("{:.2f}%",circularity);

            TextOutA(windowDc,0,0,text.c_str(), static_cast<int>(text.size()));
        }


        SelectObject(windowDc,oldPen);
        SelectObject(windowDc,oldBrush);

    }
};


int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
try
{
    auto config = INITCOMMONCONTROLSEX
    {
        .dwSize = sizeof(INITCOMMONCONTROLSEX),
        .dwICC = ICC_STANDARD_CLASSES
    };

    auto b = InitCommonControlsEx(&config);

    auto window = CircleWindow{}; 


    Window::messageLoop();
}
catch(std::exception const &e)
{
    std::print("\n{} caught \"{}\"\n",__func__,e.what());
}






