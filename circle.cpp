#include "include/window.h"

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



    bool proc(UINT m, WPARAM w, LPARAM l) override
    {
        switch(m)
        {
        case WM_LBUTTONDOWN:
            points.clear();
            points.reserve(1000);
            drawing=true;
            done=false;
            InvalidateRect(window,nullptr,true);

            return true;

        case WM_LBUTTONUP:
        {
            drawing=false;

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

            auto variance = sq_sum / lengths.size()-1;

            std::print("variance {}\n",variance);                

            std::print("{}%\n",  100 -   (100.0*sqrt(variance)/ averageLen));





            done=true;

            InvalidateRect(window,nullptr,false);

            return true;
        }

        case WM_MOUSEMOVE:
        {
            if(drawing)
            {
                auto x = LOWORD(l);         
                auto y = HIWORD(l);

                points.emplace_back(x,y);

                InvalidateRect(window,nullptr,false);
            }

            return true;
        }


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
        }
    }

};


int main()
try
{
    auto window = CircleWindow{}; 

    Window::messageLoop();
}
catch(std::exception const &e)
{
    std::print("\n{} caught \"{}\"\n",__func__,e.what());
}