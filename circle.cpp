#include "include/window.h"

#include <print>

class CircleWindow : public Window
{
public:
    CircleWindow() : Window("Circle")
    {
    }


private:

    bool                drawing{};
    std::vector<POINT>  points;


    bool proc(UINT m, WPARAM w, LPARAM l) override
    {
        switch(m)
        {
        case WM_LBUTTONDOWN:
            points.clear();
            points.reserve(1000);
            drawing=true;
            return true;

        case WM_LBUTTONUP:
            drawing=false;
            return true;


        case WM_MOUSEMOVE:
        {
            if(drawing)
            {
                auto x = LOWORD(l);         
                auto y = HIWORD(l);

                std::print("{},{}\n",x,y);

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
        auto pen = GetStockObject(BLACK_PEN);
        auto oldPen = SelectObject(windowDc,pen);

        for(auto const &point : points)
        {
            Ellipse(windowDc, 
                    point.x-1,    
                    point.y-1,
                    point.x+1,
                    point.y+1);
        }

        SelectObject(windowDc,oldPen);

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