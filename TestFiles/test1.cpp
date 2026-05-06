
struct nwocg_ExtPort { const char* name; double* val = nullptr; bool in_port;};
double add(double a, double b){ return a + b; }
double mul(double a, double b){ return a * b; }
double neg(double a){ return -a; }

struct {
    double setpoint;
    double feedback;
    double command;
    double UnitDelay1;
} nwocg; 
void nwocg_generated_init()
{
    nwocg.UnitDelay1 = 5;
}
void nwocg_generated_step()
{
    double var0 = add(neg(nwocg.feedback),nwocg.setpoint);
    double var1 = add(nwocg.UnitDelay1,mul(0.01,mul(var0,2)));
    double commandtmp = add(var1,mul(3,var0));
    double UnitDelay1tmp = var1;
    nwocg.command = commandtmp;
    nwocg.UnitDelay1 = UnitDelay1tmp;
}
static const nwocg_ExtPort
    ext_ports[] =
    {
        {"setpoint",&nwocg.setpoint,1},
        {"feedback",&nwocg.feedback,1},
        {"command",&nwocg.command,0},
        { 0, 0, 0 },
    };

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;
const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);