
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
    std::vector<Point>      rawPoints;

    bool                    done{};

    Point                   center;
    double                  circularity;
    

    void RegisterRawMouse() 
    {
        auto registration =  RAWINPUTDEVICE 
        {
            .usUsagePage = 0x01,
            .usUsage     = 0x02,
            .dwFlags     = 0,   
            .hwndTarget  = window
        };

        RegisterRawInputDevices(&registration, 1, sizeof(registration));
    }

    void lButtonDown()
    {
        points.clear();
        points.reserve(1000);

        rawPoints.clear();
        rawPoints.reserve(1000);


        auto here = POINT{};
        GetCursorPos(&here);
        ScreenToClient(window,&here);


        rawPoints.emplace_back(here.x,here.y);


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


        center.first  = static_cast<int>(std::ranges::fold_left(points |  std::ranges::views::keys ,  0, std::plus<>()) / points.size());
        center.second = static_cast<int>(std::ranges::fold_left(points |  std::ranges::views::values ,0, std::plus<>()) / points.size());

        auto lengths = std::vector<double>{};

        for(auto const &point : points)
        {
            lengths.push_back(std::hypot(point.first  - center.first,point.second - center.second));  
        }


        auto averageLen = std::ranges::fold_left(lengths ,  0.0, std::plus<>()) / lengths.size();

        std::print("averageLen {}\n",averageLen);                
        

        auto sq_sum = std::ranges::fold_left(lengths, 0.0, [averageLen](double acc, double val) {
            double diff = val - averageLen;
            return acc + (diff * diff);
        });

        auto variance = sq_sum / (lengths.size()-1);
        circularity = 100.0 -  (100.0*sqrt(variance)/ averageLen);

        std::print("variance {}\n",variance);                

        std::print("{:.1}%\n",  circularity);

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


    void input(LPARAM l)
    {
        if(!drawing)
        {
            return;
        }


        auto dataSize = UINT{};

        GetRawInputData((HRAWINPUT)l, RID_INPUT, nullptr, &dataSize, sizeof(RAWINPUTHEADER));
        
        if (dataSize > 0) 
        {
            auto data = SizedStruct<RAWINPUT>{dataSize};

            if (GetRawInputData((HRAWINPUT)l, RID_INPUT, data.bytes(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize) 
            {

                if (data->header.dwType == RIM_TYPEMOUSE) 
                {
                    auto const &mouse = data->data.mouse;

                    // Check flags to determine how coordinates are formatted
                    auto flags = mouse.usFlags;
                    
                    if ((flags & MOUSE_MOVE_ABSOLUTE) == 0) 
                    {
                        // standard behavior: Relative movement delta values
                        auto deltaX = mouse.lLastX;
                        auto deltaY = mouse.lLastY;
                        
                        rawPoints.emplace_back( rawPoints.back().first + deltaX,
                                                rawPoints.back().second + deltaY);


                    } else 
                    {
                        // Rare behavior: Absolute screen coordinates (e.g., drawing tablets/remotes)
                        LONG absX = data->data.mouse.lLastX;
                        LONG absY = data->data.mouse.lLastY;
                    }
                }
            }
        }
    }



    void SetupWindow()
    {
        Window::center();
        //RegisterRawMouse();
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

        case WM_INPUT: 
            input(l);
            return false;
        }

        return false;
    }



    void paint(PAINTSTRUCT const &, HDC windowDc)  override
    {
        auto oldPen   = SelectObject(windowDc,blackPen);

        for(auto const &point : points)
        {
            Ellipse(windowDc, 
                    point.first-1,    
                    point.second-1,
                    point.first+1,
                    point.second+1);
        }

        SelectObject(windowDc,oldPen);

        if(done)
        {
            SelectObject(windowDc,redPen);

            Ellipse(windowDc, 
                    center.first-1,    
                    center.second-1,
                    center.first+1,
                    center.second+1);

            auto text = std::format("{:.2f}%",circularity);

            TextOutA(windowDc,0,0,text.c_str(), text.size());
        }
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






