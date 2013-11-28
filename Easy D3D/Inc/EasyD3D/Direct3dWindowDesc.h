#pragma once
#include "WindowDesc.h"

enum class MSAA { None, _2X, _4X, _8X, _16X };

class Direct3dWindowDesc : public WindowDesc
{
    friend class Direct3dWindow;

protected:
    bool vSync = true;
    MSAA msaa = MSAA::_4X;

public:
    // 생성될 Direct3d 윈도우의 수직동기화 여부를 명시적으로 지정한다.
    // 지정하지 않을경우의 기본값은 true 이다.
    // VSync : 수직동기화 여부
    void VSyncStatus(bool VSync) { vSync = VSync; }

    // 생성될 Direct3d 윈도우의 멀티샘플링 품질을 명시적으로 지정한다.
    // 지정하지 않을경우, 4x MSAA 가 기본값으로 설정된다.
    // Msaa : 멀티샘플링
    void MultisamplingQuality(MSAA MSAA) { msaa = MSAA; }
};
