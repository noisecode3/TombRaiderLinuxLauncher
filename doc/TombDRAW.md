# TombDRAW - ddraw to OpenGL for Tomb Raider 4 and trle levels

A box means that the function name used to be call from tomb4.exe,
called internally comming from tomb4.exe.

This is an incomplete list for now, what we have is from ddraw.c and device.c

- [ ] DirectDrawCreate
- DirectDrawCreateClipper
- DirectDrawCreateEx
- [ ] DirectDrawEnumerateA
- [ ] DirectDrawEnumerateExA
- DirectDrawEnumerateExW
- DirectDrawEnumerateW

## IDirectDraw7

- IUnknown
  - [ ] ddraw7_QueryInterface
  - ddraw7_AddRef
  - [ ] ddraw7_Release
- IDirectDraw
  - ddraw7_Compact
  - ddraw7_CreateClipper
  - ddraw7_CreatePalette
  - ddraw7_CreateSurface
  - ddraw7_DuplicateSurface
  - [ ] ddraw7_EnumDisplayModes
  - ddraw7_EnumSurfaces
  - ddraw7_FlipToGDISurface
  - [ ] ddraw7_GetCaps
  - ddraw7_GetDisplayMode
  - [ ] ddraw7_GetFourCCCodes
  - ddraw7_GetGDISurface
  - ddraw7_GetMonitorFrequency
  - ddraw7_GetScanLine
  - ddraw7_GetVerticalBlankStatus
  - [ ] ddraw7_Initialize
  - [ ] ddraw7_RestoreDisplayMode
  - [ ] ddraw7_SetCooperativeLevel
  - [ ] ddraw7_SetDisplayMode
  - ddraw7_WaitForVerticalBlank
- IDirectDraw2
  - [ ] ddraw7_GetAvailableVidMem
- IDirectDraw3
  - ddraw7_GetSurfaceFromDC
- IDirectDraw4
  - ddraw7_RestoreAllSurfaces
  - ddraw7_TestCooperativeLevel
  - [ ] ddraw7_GetDeviceIdentifier
- IDirectDraw7
  - ddraw7_StartModeTest
  - ddraw7_EvaluateMode

## IDirectDraw4

- IUnknown
  - [ ] ddraw4_QueryInterface
  - [ ] ddraw4_AddRef
  - [ ] ddraw4_Release
- IDirectDraw
  - ddraw4_Compact
  - ddraw4_CreateClipper
  - ddraw4_CreatePalette
  - [ ] ddraw4_CreateSurface
  - ddraw4_DuplicateSurface
  - [ ] ddraw4_EnumDisplayModes
  - ddraw4_EnumSurfaces
  - ddraw4_FlipToGDISurface
  - [ ] ddraw4_GetCaps
  - ddraw4_GetDisplayMode
  - ddraw4_GetFourCCCodes
  - ddraw4_GetGDISurface
  - ddraw4_GetMonitorFrequency
  - ddraw4_GetScanLine
  - ddraw4_GetVerticalBlankStatus
  - ddraw4_Initialize
  - ddraw4_RestoreDisplayMode
  - [ ] ddraw4_SetCooperativeLevel
  - [ ] ddraw4_SetDisplayMode
  - ddraw4_WaitForVerticalBlank
- IDirectDraw2
  - ddraw4_GetAvailableVidMem
- IDirectDraw3
  - ddraw4_GetSurfaceFromDC
- IDirectDraw4
  - ddraw4_RestoreAllSurfaces
  - ddraw4_TestCooperativeLevel
  - [ ] ddraw4_GetDeviceIdentifier

## IDirectDraw2

- IUnknown
  - ddraw2_QueryInterface
  - ddraw2_AddRef
  - ddraw2_Release
- IDirectDraw
  - ddraw2_Compact
  - ddraw2_CreateClipper
  - ddraw2_CreatePalette
  - ddraw2_CreateSurface
  - ddraw2_DuplicateSurface
  - ddraw2_EnumDisplayModes
  - ddraw2_EnumSurfaces
  - ddraw2_FlipToGDISurface
  - ddraw2_GetCaps
  - ddraw2_GetDisplayMode
  - ddraw2_GetFourCCCodes
  - ddraw2_GetGDISurface
  - ddraw2_GetMonitorFrequency
  - ddraw2_GetScanLine
  - ddraw2_GetVerticalBlankStatus
  - ddraw2_Initialize
  - ddraw2_RestoreDisplayMode
  - ddraw2_SetCooperativeLevel
  - ddraw2_SetDisplayMode
  - ddraw2_WaitForVerticalBlank
- IDirectDraw2
  - ddraw2_GetAvailableVidMem

## IDirectDraw

- IUnknown
  - [ ] ddraw1_QueryInterface
  - [ ] ddraw1_AddRef
  - [ ] ddraw1_Release
- IDirectDraw
  - ddraw1_Compact
  - ddraw1_CreateClipper
  - ddraw1_CreatePalette
  - ddraw1_CreateSurface
  - ddraw1_DuplicateSurface
  - ddraw1_EnumDisplayModes
  - ddraw1_EnumSurfaces
  - ddraw1_FlipToGDISurface
  - ddraw1_GetCaps
  - ddraw1_GetDisplayMode
  - ddraw1_GetFourCCCodes
  - ddraw1_GetGDISurface
  - ddraw1_GetMonitorFrequency
  - ddraw1_GetScanLine
  - ddraw1_GetVerticalBlankStatus
  - [ ] ddraw1_Initialize
  - ddraw1_RestoreDisplayMode
  - ddraw1_SetCooperativeLevel
  - ddraw1_SetDisplayMode
  - ddraw1_WaitForVerticalBlank

## IDirect3D7

- IUnknown
  - d3d7_QueryInterface
  - d3d7_AddRef
  - d3d7_Release
- IDirect3D7
  - d3d7_EnumDevices
  - d3d7_CreateDevice
  - d3d7_CreateVertexBuffer
  - [ ] d3d7_EnumZBufferFormats
  - d3d7_EvictManagedTextures

## IDirect3D3

- IUnknown
  - d3d3_QueryInterface
  - [ ] d3d3_AddRef
  - [ ] d3d3_Release
- IDirect3D3
  - d3d3_EnumDevices
  - d3d3_CreateLight
  - d3d3_CreateMaterial
  - [ ] d3d3_CreateViewport
  - d3d3_FindDevice
  - [ ] d3d3_CreateDevice
  - [ ] d3d3_CreateVertexBuffer
  - [ ] d3d3_EnumZBufferFormats
  - d3d3_EvictManagedTextures

## IDirect3D2

- IUnknown
  - d3d2_QueryInterface
  - d3d2_AddRef
  - d3d2_Release
- IDirect3D2
  - d3d2_EnumDevices
  - d3d2_CreateLight
  - d3d2_CreateMaterial
  - d3d2_CreateViewport
  - d3d2_FindDevice
  - d3d2_CreateDevice

## IDirect3D

- IUnknown
  - d3d1_QueryInterface
  - d3d1_AddRef
  - d3d1_Release
- IDirect3D
  - d3d1_Initialize
  - d3d1_EnumDevices
  - d3d1_CreateLight
  - d3d1_CreateMaterial
  - d3d1_CreateViewport
  - d3d1_FindDevice

## IDirect3DDevice7

- IUnknown
  - d3d_device7_QueryInterface
  - d3d_device7_AddRef
  - d3d_device7_Release
- IDirect3DDevice7
  - d3d_device7_GetCaps_FPUSetup
  - d3d_device7_GetCaps_FPUPreserve
  - d3d_device7_EnumTextureFormats_FPUSetup
  - d3d_device7_EnumTextureFormats_FPUPreserve
  - d3d_device7_BeginScene_FPUSetup
  - d3d_device7_BeginScene_FPUPreserve
  - d3d_device7_EndScene_FPUSetup
  - d3d_device7_EndScene_FPUPreserve
  - d3d_device7_GetDirect3D
  - d3d_device7_SetRenderTarget_FPUSetup
  - d3d_device7_SetRenderTarget_FPUPreserve
  - d3d_device7_GetRenderTarget
  - d3d_device7_Clear_FPUSetup
  - d3d_device7_Clear_FPUPreserve
  - d3d_device7_SetTransform_FPUSetup
  - d3d_device7_SetTransform_FPUPreserve
  - d3d_device7_GetTransform_FPUSetup
  - d3d_device7_GetTransform_FPUPreserve
  - d3d_device7_SetViewport_FPUSetup
  - d3d_device7_SetViewport_FPUPreserve
  - d3d_device7_MultiplyTransform_FPUSetup
  - d3d_device7_MultiplyTransform_FPUPreserve
  - d3d_device7_GetViewport_FPUSetup
  - d3d_device7_GetViewport_FPUPreserve
  - d3d_device7_SetMaterial_FPUSetup
  - d3d_device7_SetMaterial_FPUPreserve
  - d3d_device7_GetMaterial_FPUSetup
  - d3d_device7_GetMaterial_FPUPreserve
  - d3d_device7_SetLight_FPUSetup
  - d3d_device7_SetLight_FPUPreserve
  - d3d_device7_GetLight_FPUSetup
  - d3d_device7_GetLight_FPUPreserve
  - d3d_device7_SetRenderState_FPUSetup
  - d3d_device7_SetRenderState_FPUPreserve
  - d3d_device7_GetRenderState_FPUSetup
  - d3d_device7_GetRenderState_FPUPreserve
  - d3d_device7_BeginStateBlock_FPUSetup
  - d3d_device7_BeginStateBlock_FPUPreserve
  - d3d_device7_EndStateBlock_FPUSetup
  - d3d_device7_EndStateBlock_FPUPreserve
  - d3d_device7_PreLoad_FPUSetup
  - d3d_device7_PreLoad_FPUPreserve
  - d3d_device7_DrawPrimitive_FPUSetup
  - d3d_device7_DrawPrimitive_FPUPreserve
  - d3d_device7_DrawIndexedPrimitive_FPUSetup
  - d3d_device7_DrawIndexedPrimitive_FPUPreserve
  - d3d_device7_SetClipStatus
  - d3d_device7_GetClipStatus
  - d3d_device7_DrawPrimitiveStrided_FPUSetup
  - d3d_device7_DrawPrimitiveStrided_FPUPreserve
  - d3d_device7_DrawIndexedPrimitiveStrided_FPUSetup
  - d3d_device7_DrawIndexedPrimitiveStrided_FPUPreserve
  - d3d_device7_DrawPrimitiveVB_FPUSetup
  - d3d_device7_DrawPrimitiveVB_FPUPreserve
  - d3d_device7_DrawIndexedPrimitiveVB_FPUSetup
  - d3d_device7_DrawIndexedPrimitiveVB_FPUPreserve
  - d3d_device7_ComputeSphereVisibility
  - d3d_device7_GetTexture_FPUSetup
  - d3d_device7_GetTexture_FPUPreserve
  - d3d_device7_SetTexture_FPUSetup
  - d3d_device7_SetTexture_FPUPreserve
  - d3d_device7_GetTextureStageState_FPUSetup
  - d3d_device7_GetTextureStageState_FPUPreserve
  - d3d_device7_SetTextureStageState_FPUSetup
  - d3d_device7_SetTextureStageState_FPUPreserve
  - d3d_device7_ValidateDevice_FPUSetup
  - d3d_device7_ValidateDevice_FPUPreserve
  - d3d_device7_ApplyStateBlock_FPUSetup
  - d3d_device7_ApplyStateBlock_FPUPreserve
  - d3d_device7_CaptureStateBlock_FPUSetup
  - d3d_device7_CaptureStateBlock_FPUPreserve
  - d3d_device7_DeleteStateBlock_FPUSetup
  - d3d_device7_DeleteStateBlock_FPUPreserve
  - d3d_device7_CreateStateBlock_FPUSetup
  - d3d_device7_CreateStateBlock_FPUPreserve
  - d3d_device7_Load_FPUSetup
  - d3d_device7_Load_FPUPreserve
  - d3d_device7_LightEnable_FPUSetup
  - d3d_device7_LightEnable_FPUPreserve
  - d3d_device7_GetLightEnable_FPUSetup
  - d3d_device7_GetLightEnable_FPUPreserve
  - d3d_device7_SetClipPlane_FPUSetup
  - d3d_device7_SetClipPlane_FPUPreserve
  - d3d_device7_GetClipPlane_FPUSetup
  - d3d_device7_GetClipPlane_FPUPreserve
  - d3d_device7_GetInfo

## IDirect3DDevice3

- IUnknown
  - d3d_device3_QueryInterface
  - d3d_device3_AddRef
  - d3d_device3_Release
- IDirect3DDevice3
  - d3d_device3_GetCaps
  - d3d_device3_GetStats
  - d3d_device3_AddViewport
  - d3d_device3_DeleteViewport
  - d3d_device3_NextViewport
  - d3d_device3_EnumTextureFormats
  - [ ] d3d_device3_BeginScene
  - d3d_device3_EndScene
  - d3d_device3_GetDirect3D
  - d3d_device3_SetCurrentViewport
  - d3d_device3_GetCurrentViewport
  - d3d_device3_SetRenderTarget
  - d3d_device3_GetRenderTarget
  - d3d_device3_Begin
  - d3d_device3_BeginIndexed
  - d3d_device3_Vertex
  - d3d_device3_Index
  - d3d_device3_End
  - d3d_device3_GetRenderState
  - [ ] d3d_device3_SetRenderState
  - d3d_device3_GetLightState
  - [ ] d3d_device3_SetLightState
  - d3d_device3_SetTransform
  - d3d_device3_GetTransform
  - d3d_device3_MultiplyTransform
  - d3d_device3_DrawPrimitive
  - d3d_device3_DrawIndexedPrimitive
  - d3d_device3_SetClipStatus
  - d3d_device3_GetClipStatus
  - d3d_device3_DrawPrimitiveStrided
  - d3d_device3_DrawIndexedPrimitiveStrided
  - d3d_device3_DrawPrimitiveVB
  - d3d_device3_DrawIndexedPrimitiveVB
  - d3d_device3_ComputeSphereVisibility
  - d3d_device3_GetTexture
  - d3d_device3_SetTexture
  - d3d_device3_GetTextureStageState
  - d3d_device3_SetTextureStageState
  - d3d_device3_ValidateDevice

## IDirect3DDevice2

- IUnknown
  - d3d_device2_QueryInterface
  - d3d_device2_AddRef
  - d3d_device2_Release
- IDirect3DDevice2
  - d3d_device2_GetCaps
  - d3d_device2_SwapTextureHandles
  - d3d_device2_GetStats
  - d3d_device2_AddViewport
  - d3d_device2_DeleteViewport
  - d3d_device2_NextViewport
  - d3d_device2_EnumTextureFormats
  - d3d_device2_BeginScene
  - d3d_device2_EndScene
  - d3d_device2_GetDirect3D
  - d3d_device2_SetCurrentViewport
  - d3d_device2_GetCurrentViewport
  - d3d_device2_SetRenderTarget
  - d3d_device2_GetRenderTarget
  - d3d_device2_Begin
  - d3d_device2_BeginIndexed
  - d3d_device2_Vertex
  - d3d_device2_Index
  - d3d_device2_End
  - d3d_device2_GetRenderState
  - d3d_device2_SetRenderState
  - d3d_device2_GetLightState
  - d3d_device2_SetLightState
  - d3d_device2_SetTransform
  - d3d_device2_GetTransform
  - d3d_device2_MultiplyTransform
  - d3d_device2_DrawPrimitive
  - d3d_device2_DrawIndexedPrimitive
  - d3d_device2_SetClipStatus
  - d3d_device2_GetClipStatus

## IDirect3DDevice

- IUnknown
  - d3d_device1_QueryInterface
  - d3d_device1_AddRef
  - d3d_device1_Release
- IDirect3DDevice1
  - d3d_device1_Initialize
  - d3d_device1_GetCaps
  - d3d_device1_SwapTextureHandles
  - d3d_device1_CreateExecuteBuffer
  - d3d_device1_GetStats
  - d3d_device1_Execute
  - d3d_device1_AddViewport
  - d3d_device1_DeleteViewport
  - d3d_device1_NextViewport
  - d3d_device1_Pick
  - d3d_device1_GetPickRecords
  - d3d_device1_EnumTextureFormats
  - d3d_device1_CreateMatrix
  - d3d_device1_SetMatrix
  - d3d_device1_GetMatrix
  - d3d_device1_DeleteMatrix
  - d3d_device1_BeginScene
  - d3d_device1_EndScene
  - d3d_device1_GetDirect3D
