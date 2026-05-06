
struct nwocg_ExtPort { const char* name; double* val = nullptr; bool in_port;};
double add(double a, double b){ return a + b; }
double mul(double a, double b){ return a * b; }
double neg(double a){ return -a; }

struct {
    double Out1;
    double Out2;
    double In2;
    double Out3;
    double In1;
    double UnitDelay1;
    double UnitDelay;
} nwocg; 
void nwocg_generated_init()
{
    nwocg.UnitDelay1 = 0;
    nwocg.UnitDelay = 0;
}
void nwocg_generated_step()
{
    double var0 = add(add(nwocg.In2,neg(nwocg.UnitDelay)),nwocg.In2);
    double Out1tmp = add(nwocg.UnitDelay1,add(nwocg.In2,nwocg.In1));
    double Out2tmp = var0;
    double Out3tmp = nwocg.UnitDelay1;
    double UnitDelaytmp = nwocg.UnitDelay1;
    double UnitDelay1tmp = var0;
    nwocg.Out1 = Out1tmp;
    nwocg.Out2 = Out2tmp;
    nwocg.Out3 = Out3tmp;
    nwocg.UnitDelay = UnitDelaytmp;
    nwocg.UnitDelay1 = UnitDelay1tmp;
}
static const nwocg_ExtPort
    ext_ports[] =
    {
        {"Out1",&nwocg.Out1,0},
        {"Out2",&nwocg.Out2,0},
        {"In2",&nwocg.In2,1},
        {"Out3",&nwocg.Out3,0},
        {"In1",&nwocg.In1,1},
        { 0, 0, 0 },
    };

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;
const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);