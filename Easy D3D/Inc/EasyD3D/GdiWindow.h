#pragma once
#include "Window.h"

// GDI 객체로 그림을 그릴 수 있는 Window를 생성한다
class GdiWindow : public Window
{
private:
    std::function<void(HDC)> draw;
    
public:
    // 새로운 GdiWindow 클래스의 인스턴스를 생성한다.
    // Description : 윈도우 생성에 쓰일 설정
    GdiWindow(const WindowDesc& Description) : Window(Description) { }

    // 그리기를 수행할 콜백함수를 정한다
    // void Callback(HDC hdc) : 매 렌더링 시점마다 호출될 콜백함수
    //     hdc : 렌더링에 사용될 DC 핸들
    void setDraw(const decltype(draw)& Callback) { draw = Callback; }
    // 그리기를 수행할 콜백함수를 정한다
    // void Callback(HDC hdc) : 매 렌더링 시점마다 호출될 콜백함수
    //     hdc : 렌더링에 사용될 DC 핸들
    void setDraw(decltype(draw)&& Callback) { draw = Callback; }

protected:
    virtual void onDraw() override;
};
