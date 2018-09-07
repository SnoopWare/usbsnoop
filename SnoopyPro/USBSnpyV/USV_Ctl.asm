
;*****************************************************************************
;
; Module: 
; USV_Ctl.asm
;
; Description: 
; USB Snoopies entry point
;
;*****************************************************************************

        .386p

        .xlist

        include vmm.inc
        include debug.inc

EXTRN STDCALL USBSnoopies_OnLoad@0:PROC
EXTRN STDCALL USBSnoopies_OnUnload@0:PROC

;============================================================================
;               V I R T U A L   D E V I C E   D E C L A R A T I O N
;============================================================================


DECLARE_VIRTUAL_DEVICE USBSNPYV,0,20,USBSnoopies_Control,UNDEFINED_DEVICE_ID,UNDEFINED_INIT_ORDER


VxD_LOCKED_CODE_SEG

BeginProc USBSnoopies_Control
        Control_Dispatch W32_DEVICEIOCONTROL, USBSnoopies_DeviceIOCTL, sCall, <ecx, ebx, edx, esi>
        Control_Dispatch SYS_DYNAMIC_DEVICE_INIT, USBSnoopies_DeviceInit
        Control_Dispatch SYS_DYNAMIC_DEVICE_EXIT, USBSnoopies_DeviceExit
        clc                                     ; succeed
        ret
EndProc USBSnoopies_Control

BeginProc USBSnoopies_DeviceInit
        call    USBSnoopies_OnLoad@0
		mov		eax, VXD_SUCCESS
        clc                                     ; succeed
        ret
EndProc USBSnoopies_DeviceInit

BeginProc USBSnoopies_DeviceExit
        call    USBSnoopies_OnUnload@0
		mov		eax, VXD_SUCCESS
        clc                                     ; succeed
        ret
EndProc USBSnoopies_DeviceExit

VXD_LOCKED_CODE_ENDS

end
