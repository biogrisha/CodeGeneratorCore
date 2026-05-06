
struct nwocg_ExtPort { const char* name; double* val = nullptr; bool in_port;};
double add(double a, double b){ return a + b; }
double mul(double a, double b){ return a * b; }
double neg(double a){ return -a; }

struct {
    double Out1;
    double In2;
    double In1;
    double UnitDelay;
} nwocg; 
void nwocg_generated_init()
{
    nwocg.UnitDelay = 0;
}
void nwocg_generated_step()
{
    double var0 = add(add(neg(add(add(nwocg.In2,neg(nwocg.UnitDelay)),nwocg.In2)),nwocg.In2),nwocg.In1);
    double Out1tmp = var0;
    double UnitDelaytmp = var0;
    nwocg.Out1 = Out1tmp;
    nwocg.UnitDelay = UnitDelaytmp;
}
static const nwocg_ExtPort
    ext_ports[] =
    {
        {"Out1",&nwocg.Out1,0},
        {"In2",&nwocg.In2,1},
        {"In1",&nwocg.In1,1},
        { 0, 0, 0 },
    };

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;
const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);