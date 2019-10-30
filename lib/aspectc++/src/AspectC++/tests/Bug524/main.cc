pointcut asEnableAllInterrupts () = "% AS::EnableAllInterrupts (...)";
pointcut asInterruptServices () = asEnableAllInterrupts ();

aspect os_dep_KernelStructsSupport {
    advice call (asInterruptServices ()) : order (
         "os_krn_EnterLeave_Weaving_Static",
         "os_dep_KernelStructsSupport",
         ! "os_dep_KernelStructsSupport"&&
!"os_krn_EnterLeave_Weaving_Static"
    );
};

int main() {}
