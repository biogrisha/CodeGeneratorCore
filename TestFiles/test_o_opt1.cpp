
struct nwocg_ExtPort { const char* name; double* val = nullptr; bool in_port;};
double add(double a, double b){ return a + b; }
double mul(double a, double b){ return a * b; }
double neg(double a){ return -a; }

struct {
    double command;
    double setpoint;
    double feedback;
    double UnitDelay1;
} nwocg; 
void nwocg_generated_init()
{
    nwocg.UnitDelay1 = 5;
}
void nwocg_generated_step()
{
    double var0 = add(nwocg.setpoint,neg(nwocg.feedback));
    double var1 = add(mul(mul(var0,2),0.01),nwocg.UnitDelay1);
    double commandtmp = add(mul(var0,3),var1);
    double UnitDelay1tmp = var1;
    nwocg.command = commandtmp;
    nwocg.UnitDelay1 = UnitDelay1tmp;
}
static const nwocg_ExtPort
    ext_ports[] =
    {
        {"command",&nwocg.command,0},
        {"setpoint",&nwocg.setpoint,1},
        {"feedback",&nwocg.feedback,1},
        { 0, 0, 0 },
    };

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;
const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);