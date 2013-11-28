#pragma once
#include <dxgi.h>
#include <d3d11.h>
#include <EasyD3D/Utility.h>
#include "Window.h"
class Direct3dWindowDesc;

// Direct3D로 화면을 렌더링하는 Window를 생성한다
class Direct3dWindow : public Window
{
private:
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<IDXGISwapChain> swapChain;

    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ComPtr<ID3D11DepthStencilView> depthStencilView;

    const bool vSync;

    std::function<void(ComPtr<ID3D11DeviceContext>)> draw;

public:
    // 새로운 Window 클래스의 인스턴스를 생성한다.
    // Description : 윈도우 생성에 쓰일 설정
    Direct3dWindow(const Direct3dWindowDesc& Description);

    virtual ~Direct3dWindow();

    // 그리기를 수행할 콜백함수를 정한다
    // void Callback() : 매 렌더링 시점마다 호출될 콜백함수
    //     ComPtr<ID3D11Device> : 그리기에 사용될 Direct3D Device Context
    void setDraw(const decltype(draw)& Callback) { draw = Callback; }
    // 그리기를 수행할 콜백함수를 정한다
    // void Callback() : 매 렌더링 시점마다 호출될 콜백함수
    //     ComPtr<ID3D11Device> : 그리기에 사용될 Direct3D Device Context
    void setDraw(decltype(draw)&& Callback) { draw = Callback; }

    // Direct3dWindow 클래스에 의해 생성된 Direct3D11 Device를 반환한다
    // 반환값 : Direct3D11 Device
    ComPtr<ID3D11Device> getDevice() const { return device; }

protected:
    virtual void onDraw() override;
    virtual void onResize(ushort Width, ushort Height) override;
    virtual void onEnterFullscreenMode() override;
    virtual void onLeaveFullscreenMode() override;
};
