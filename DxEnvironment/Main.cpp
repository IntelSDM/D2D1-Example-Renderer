// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>
#include <float.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <codecvt>
#include <locale>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite")
ID2D1Factory* Factory;             
ID2D1HwndRenderTarget* RenderTarget;
IDWriteFactory* FontFactory;
ID2D1SolidColorBrush* Brush;
ID2D1LinearGradientBrush* LinearBrush;
ID2D1GradientStopCollection* GradientStops = NULL;




IDWriteTextFormat* Fonts[6]; // could replace this with a map but thats effort
int FontNumber;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void CreateFonts(std::wstring_view fontname, float size, DWRITE_FONT_WEIGHT weight);







void CreateLinearGradientBrush()
{
  
    D2D1_GRADIENT_STOP stops[] =
    {
        { 0.0f, D2D1::ColorF(D2D1::ColorF::Cyan) },
        { 1.0f, D2D1::ColorF(D2D1::ColorF::DarkBlue) }
    };

    ID2D1GradientStopCollection* collection;

   

    RenderTarget->CreateGradientStopCollection(
        stops,
        _countof(stops),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &GradientStops
    );


    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props = {};

    RenderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(0, 0),
            D2D1::Point2F(150, 150)),
        GradientStops,
        &LinearBrush
    );
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    case WM_PAINT:



        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}



void InitD2D(HWND hWnd)
{

  
    HRESULT result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &Factory);


   
    RECT rect;
    GetClientRect(hWnd, &rect);
    result = Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(rect.right, rect.bottom)), &RenderTarget);
    if (SUCCEEDED(result))
    {
        result = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&FontFactory)
        );
    }
    if (SUCCEEDED(result))
    {
  
        CreateFonts(L"Verdana" ,12, DWRITE_FONT_WEIGHT_REGULAR);
        CreateFonts(L"Courier New", 15, DWRITE_FONT_WEIGHT_BOLD);

        CreateFonts(L"Tahoma", 16, DWRITE_FONT_WEIGHT_REGULAR);
        

      
    }
    if (SUCCEEDED(result))
    {
        // we do create these constantly but we need to make sure the stops and brush aren't a nullptr or we cant release it until we draw with it
        CreateLinearGradientBrush();
        RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0,0,0,0), &Brush);

        RenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
        
    }
  

}

D2D1::ColorF Colour(UINT8 R, UINT8 G, UINT8 B, UINT8 A)
{
    return D2D1::ColorF(static_cast<float>(R) / 255.0f, static_cast<float>(G) / 255.0f, static_cast<float>(B) / 255.0f, static_cast<float>(A) / 255.0f);

}





void CreateFonts(std::wstring_view fontname, float size, DWRITE_FONT_WEIGHT weight)
{
  
  
    IDWriteTextFormat* text_format;
    
    HRESULT result = FontFactory->CreateTextFormat(
        fontname.data(),                
        nullptr,                     
        weight,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        size,
        L"",
        &Fonts[FontNumber++]

       
    );
  

}
static std::wstring string_to_wstring(const std::string input)
{
    std::wstring wstr(input.begin(), input.end());
    return wstr;
}
void GetTextSize(const std::wstring_view text,float* const width,float* const height, int font)
{
    if (!text.empty()) 
    {
        if (!width && !height) 
        {
            return;
        }

        IDWriteTextLayout* layout = nullptr;
        const HRESULT status = FontFactory->CreateTextLayout(text.data(),static_cast<std::uint32_t>(text.length()),Fonts[font],4096.f, 4096.f,&layout);

        if (SUCCEEDED(status)) 
        {
            DWRITE_TEXT_METRICS metrics{};
            if (SUCCEEDED(layout->GetMetrics(&metrics)))
            {
                if (width) 
                {
                    *width = metrics.width;
                }
                if (height) 
                {
                    *height = metrics.height;
                }
            }
            layout->Release();
        }
    }
}

void GetTextSize(const std::string text,float* const width,float* const height,int font)
{
    if (!text.empty()) 
    {
        GetTextSize(string_to_wstring(text), width, height,font);
    }
}





/// <summary>
/// Draws text
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
void Text(std::wstring_view text,int x,int y, D2D1::ColorF colour, int font, bool centred)
{


    RenderTarget->CreateSolidColorBrush(colour, &Brush);
  
    if (centred)
    {
        // you can also centre y axis as well  by doing y - (rect2.bottom / 2) in the y cast
        //flt max means the chars have no limit
        D2D1_RECT_F rect2 = { static_cast<float>(0), static_cast<float>(0), FLT_MAX, FLT_MAX };
        GetTextSize(text, &rect2.right, &rect2.bottom, font);
        D2D1_RECT_F rect = { static_cast<float>(x - (rect2.right / 2)), static_cast<float>(y), FLT_MAX, FLT_MAX };

        RenderTarget->DrawTextW(text.data(), static_cast<std::uint32_t>(text.length()), Fonts[font], &rect, Brush);
    }
    else
    {
        D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), FLT_MAX, FLT_MAX };
        RenderTarget->DrawTextW(text.data(), static_cast<std::uint32_t>(text.length()), Fonts[font], &rect, Brush);
        
    }

    /*
    * // this allows us to debug values
    std::string test = std::to_string(rect.right);
    std::wstring wstr = string_to_wstring(test);
    const wchar_t* szName = wstr.c_str();
    */
    
 

}
void Text(std::string text, int x, int y, D2D1::ColorF colour, int font, bool centred)
{

   
        RenderTarget->CreateSolidColorBrush(colour, &Brush);

    if (centred)
    {
        
        D2D1_RECT_F rect2 = { static_cast<float>(0), static_cast<float>(0), FLT_MAX, FLT_MAX };
        GetTextSize(text, &rect2.right, &rect2.bottom, font);
        D2D1_RECT_F rect = { static_cast<float>(x - (rect2.right / 2)), static_cast<float>(y), FLT_MAX, FLT_MAX };

        RenderTarget->DrawTextW(string_to_wstring(text).data(), static_cast<std::uint32_t>(string_to_wstring(text).length()), Fonts[font], &rect, Brush);
    }
    else
    {
        D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), FLT_MAX, FLT_MAX };
        RenderTarget->DrawTextW(string_to_wstring(text).data(), static_cast<std::uint32_t>(string_to_wstring(text).length()), Fonts[font], &rect, Brush);

    }

   



}
/// <summary>
/// Draws filled and normal circles
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="colour"></param>
/// <param name="radius"></param>
/// <param name="filled"></param>
/// <param name="width"></param>
void Circle(int x,int y, D2D1::ColorF colour,float radius,bool filled,float width = 1)
{
    if (!filled)
    {
        RenderTarget->CreateSolidColorBrush(colour, &Brush);
        RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        RenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), Brush, width);
        RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    }
    else
    {
        RenderTarget->CreateSolidColorBrush(colour, &Brush);
        RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        RenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), Brush);
        RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

    }

}
/// <summary>
/// Line Rectangle
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="colour"></param>
/// <param name="x"></param>
/// <param name="linewidth"></param>
void Rectangle(int x, int y, int width, int height, int linewidth ,D2D1::ColorF colour)
{
 
    RenderTarget->CreateSolidColorBrush(colour, &Brush);
    D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(width + x), static_cast<float>(height + y) };
    RenderTarget->DrawRectangle(rect, Brush,linewidth);
   
}
/// <summary>
/// Filled Rectangle
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="linewidth"></param>
/// <param name="colour"></param>
void Rectangle(int x, int y, int width, int height, D2D1::ColorF colour)
{
   
    //  RenderTarget.recta
    RenderTarget->CreateSolidColorBrush(colour, &Brush);
    D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(width + x), static_cast<float>(height + y) };
    RenderTarget->FillRectangle(rect, Brush);
  
    //RenderTarget->DrawRectangle(rect, Brush);
}
/// <summary>
/// 
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="linewidth"></param>
/// <param name="colour"></param>
/// <param name="aa">Anti Aliasing On The Corners (Rounding)</param>
void RoundedRectangle(int x, int y, int width, int height, int linewidth, D2D1::ColorF colour,int aa = 10)
{
   
    
    RenderTarget->CreateSolidColorBrush(colour, &Brush);
    D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(width + x), static_cast<float>(height + y) };
    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
        rect,
        static_cast<float>(aa),
        static_cast<float>(aa)
    );
    RenderTarget->DrawRoundedRectangle(roundedRect, Brush,linewidth);

   
}
/// <summary>
/// 
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="colour"></param>
/// <param name="aa">Anti Aliasing On The Corners (Rounding)</param>
void RoundedRectangle(int x, int y, int width, int height, D2D1::ColorF colour,int aa = 10)
{
 
    //   ID2D1StrokeStyle
    RenderTarget->CreateSolidColorBrush(colour, &Brush);
    D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(width + x), static_cast<float>(height + y) };
    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
        rect,
        static_cast<float>(aa),
        static_cast<float>(aa)
    );
    RenderTarget->FillRoundedRectangle(roundedRect, Brush);


}

void Line(int xstart, int ystart, int xend, int yend, D2D1::ColorF colour, int width = 1)
{
    D2D1_POINT_2F start = { static_cast<float>(xstart), static_cast<float>(ystart) };
    D2D1_POINT_2F finish = { static_cast<float>(xend), static_cast<float>(yend) };
    RenderTarget->CreateSolidColorBrush(colour, &Brush);
    
    RenderTarget->DrawLine(start, finish, Brush,width);


}
 

void LinearRectangle(int x,int y,int width,int height, D2D1::ColorF colour1, D2D1::ColorF colour2, float point1x = 0,float point1y = 0 ,float point2x = 150,float point2y = 150)
{
    //https://i.imgur.com/fGvBnxh.png
    // https://i.imgur.com/aw70bIz.png
    D2D1_GRADIENT_STOP stops[] =
    {
        { 0.0f,colour1 },
        { 1.0f,colour2 },
        //{ 1.0f, Colour(0,0,255,255) }
    };
    ID2D1GradientStopCollection* collection;
    //https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-brushes-overview 

    RenderTarget->CreateGradientStopCollection(
        stops,
        _countof(stops),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &GradientStops
    );
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props = {};
    auto size = RenderTarget->GetSize();
    RenderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(point1x, point1y),
            D2D1::Point2F(point2x, point2y)),
        GradientStops,
        &LinearBrush
    );

    D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(width + x), static_cast<float>(height + y) };

    RenderTarget->FillRectangle(rect, LinearBrush);
    GradientStops->Release();
    LinearBrush->Release();

}
/// <summary>
/// 
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="colour1"></param>
/// <param name="colour2"></param>
/// <param name="point1x"></param>
/// <param name="point1y"></param>
/// <param name="point2x"></param>
/// <param name="point2y"></param>
/// <param name="aa">Anti Aliasing On The Corners (Rounding)</param>
void LinearRoundedRectangle(int x, int y, int width, int height, D2D1::ColorF colour1, D2D1::ColorF colour2, float point1x = 0, float point1y = 0, float point2x = 150, float point2y = 150,int aa = 10)
{
    //https://i.imgur.com/fGvBnxh.png
    // https://i.imgur.com/aw70bIz.png
    D2D1_GRADIENT_STOP stops[] =
    {
        { 0.0f,colour1 },
        { 1.0f,colour2 },
        //{ 1.0f, Colour(0,0,255,255) }
    };
    ID2D1GradientStopCollection* collection;
    //https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-brushes-overview 

    RenderTarget->CreateGradientStopCollection(
        stops,
        _countof(stops),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &GradientStops
    );
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props = {};
    auto size = RenderTarget->GetSize();
    RenderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(point1x, point1y),
            D2D1::Point2F(point2x, point2y)),
        GradientStops,
        &LinearBrush
    );

    D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(width + x), static_cast<float>(height + y) };
    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
       rect,
        static_cast<float>(aa),
        static_cast<float>(aa)
    );
    RenderTarget->FillRoundedRectangle(roundedRect, LinearBrush);
    GradientStops->Release();
    LinearBrush->Release();

}
void LinearText(std::wstring_view text, int x, int y, int font, bool centred, D2D1::ColorF colour1, D2D1::ColorF colour2, float point1x = 0, float point1y = 0, float point2x = 150, float point2y = 150)
{
    D2D1_GRADIENT_STOP stops[] =
    {
        { 0.0f,colour1 },
        { 1.0f,colour2 },
        //{ 1.0f, Colour(0,0,255,255) }
    };
    RenderTarget->CreateGradientStopCollection(
        stops,
        _countof(stops),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &GradientStops
    );
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props = {};
    D2D1_SIZE_F size = RenderTarget->GetSize();

    RenderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(point1x, point1y),
            D2D1::Point2F(point2x, point2y)),
        GradientStops,
        &LinearBrush
    );


    if (centred)
    {
        // you can also centre y axis as well  by doing y - (rect2.bottom / 2) in the y cast
        //flt max means the chars have no limit
        D2D1_RECT_F rect2 = { static_cast<float>(0), static_cast<float>(0), FLT_MAX, FLT_MAX };
        GetTextSize(text, &rect2.right, &rect2.bottom, font);
        D2D1_RECT_F rect = { static_cast<float>(x - (rect2.right / 2)), static_cast<float>(y), FLT_MAX, FLT_MAX };

        RenderTarget->DrawTextW(text.data(), static_cast<std::uint32_t>(text.length()), Fonts[font], &rect, LinearBrush);
    }
    else
    {
        D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), FLT_MAX, FLT_MAX };
        RenderTarget->DrawTextW(text.data(), static_cast<std::uint32_t>(text.length()), Fonts[font], &rect, LinearBrush);

    }
    GradientStops->Release();
    LinearBrush->Release();
   
    


}
void LinearText(std::string text, int x, int y, int font, bool centred, D2D1::ColorF colour1, D2D1::ColorF colour2, float point1x = 0, float point1y = 0, float point2x = 150, float point2y = 150)
{
    D2D1_GRADIENT_STOP stops[] =
    {
        { 0.0f,colour1 },
        { 1.0f,colour2 },
        //{ 1.0f, Colour(0,0,255,255) }
    };
    RenderTarget->CreateGradientStopCollection(
        stops,
        _countof(stops),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &GradientStops
    );
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props = {};
    D2D1_SIZE_F size = RenderTarget->GetSize();

    RenderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(point1x, point1y),
            D2D1::Point2F(point2x, point2y)),
        GradientStops,
        &LinearBrush
    );


    if (centred)
    {
        // you can also centre y axis as well  by doing y - (rect2.bottom / 2) in the y cast
        //flt max means the chars have no limit
        D2D1_RECT_F rect2 = { static_cast<float>(0), static_cast<float>(0), FLT_MAX, FLT_MAX };
        GetTextSize(text, &rect2.right, &rect2.bottom, font);
        D2D1_RECT_F rect = { static_cast<float>(x - (rect2.right / 2)), static_cast<float>(y), FLT_MAX, FLT_MAX };

        RenderTarget->DrawTextW(string_to_wstring(text).data(), static_cast<std::uint32_t>(string_to_wstring(text).length()), Fonts[font], &rect, LinearBrush);
    }
    else
    {
        D2D1_RECT_F rect = { static_cast<float>(x), static_cast<float>(y), FLT_MAX, FLT_MAX };
        RenderTarget->DrawTextW(string_to_wstring(text).data(), static_cast<std::uint32_t>(string_to_wstring(text).length()), Fonts[font], &rect, LinearBrush);

    }
    GradientStops->Release();
    LinearBrush->Release();




}
POINT MousePos;
std::string testtextshit = "Test";
void RenderFrame(void)
{
   

    RenderTarget->BeginDraw();
    std::string pos = std::to_string(MousePos.x) + " " + std::to_string(MousePos.y);

   



    RenderTarget->Clear(Colour(100,0,0,255));

    LinearRoundedRectangle(100,100,300,400,Colour(0,255,100,255),Colour(60,0,255,255),150,50, 150, 50);
    LinearText(L"Nice",100, 100,1,true ,Colour(0, 255, 100, 255), Colour(60, 0, 255, 100), 0, 0, 150, 150);
  //  RoundedRectangle(100, 100, 100, 100, Colour(100, 100, 100, 255), 10);
    Rectangle(500, 300, 200, 200, Colour(0, 0, 255, 255));
    Rectangle(500, 500, 200, 200, Colour(0, 255, 255, 255));  
    Rectangle(900, 300, 200, 200, Colour(0, 0, 255, 255));
    Rectangle(100, 500, 200, 200, Colour(0, 255, 255, 255));
    Rectangle(400, 300, 200, 200, Colour(0, 0, 255, 255));
    Rectangle(1100, 500, 200, 200, Colour(0, 255, 255, 255));

    Rectangle(500, 800, 200, 200, Colour(0, 255, 0, 255));


    Text(pos,0,0,Colour(0,255,0,255),0, false);
    Text(L"Nice", 100, 0, Colour(0, 255, 0, 255), 0, false);
    Text(L"Nice", 290, 0, Colour(0, 255, 0, 255), 0, false);
    Text(L"Nice", 540, 0, Colour(0, 255, 0, 255), 0, false);
    Text(L"Nice", 350, 0, Colour(0, 255, 0, 255), 0, false);
    Text(L"Nice", 900, 0, Colour(0, 255, 0, 255), 0, false);
    Text(L"Nice", 650, 0, Colour(0, 255, 0, 255), 0, false);
    Text(L"Nice", 1000, 100, Colour(0, 255, 0, 255), 0, false);

    RoundedRectangle(500, 500, 300, 500, Colour(0, 0, 255, 255), 10);

    Circle(RenderTarget->GetSize().width / 2, RenderTarget->GetSize().height / 2, Colour(100, 200, 0, 255), 100,true, 2);
    Text(L"Middle", RenderTarget->GetSize().width / 2, RenderTarget->GetSize().height / 2, Colour(0, 255, 0, 255), 0, true);
    Circle(RenderTarget->GetSize().width / 2, RenderTarget->GetSize().height / 2, Colour(255, 255, 255, 255), 130, false, 2);
    Circle(RenderTarget->GetSize().width / 2, RenderTarget->GetSize().height / 2, Colour(255, 255, 255, 255), 160, false, 1);

    Text(pos, MousePos.x, MousePos.y, Colour(0, 255, 100, 255), 0, false);

    RenderTarget->EndDraw();

}


void CleanD2D(void)
{
    Factory->Release();
    RenderTarget->Release();
    Brush->Release();

    for (IDWriteTextFormat* it : Fonts)
    {
        if(it != NULL)
            it->Release();
    }
       

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    // this calculates the client area
    RECT wr = { 0, 0, 1920, 1080 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL, L"WindowClass", L"D2D1 Environment", WS_OVERLAPPEDWINDOW, 0, 0, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    InitD2D(hWnd);

    MSG msg;

    while (TRUE)
    {
    

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }
        GetCursorPos(&MousePos);
        // memory leaks and high gpu and cpu usage seems to occur when its minimised so this is a check for if its not minimised
     if(!IsIconic(hWnd))
        RenderFrame();
        
    }
    CleanD2D();

    return msg.wParam;
}