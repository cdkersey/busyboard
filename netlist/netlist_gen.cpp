#include <iostream>
#include <chdl/chdl.h>

using namespace std;
using namespace chdl;

#ifdef SIM
// TODO: CHDL simulation models, including parport input.
#else
// 74hc4094 Parallel-out-serial-in shift register with tri-state outputs
void Reg4094(bus<8> &q, node &qs1, node &qs2,
	     node data, node cp, node strobe, node oe)
{
  // Pins (DIP): VDD=16, GND=8,
  // 2, 3, 1, 15,
  // 9, 10,
  // 4, 5, 6, 7, 14, 13, 12, 11
  Module("74hc4094")
    .inputs(data)(cp)(strobe)(oe)
    .outputs(qs1)(qs2)
    .inouts(q)
  ;
}

// 74hc597 Parallel-in-serial-out shift register
void Reg597(node &q7,
	    node ds, bvec<8> d, node st_cp, node sh_cp, node npl, node nmr)
{
  // Pins (DIP): VDD=16, GND=8,
  // 14, 15, 1, 2, 3, 4, 5, 6, 7, 12, 11, 13, 10,
  // 9
  Module("74hc597")
    .inputs(ds)(d)(st_cp)(sh_cp)(npl)(nmr)
    .outputs(q7)
  ;
}

// Centronics printer port (36-pin; previously-common printer-side connector)
void Centronics(
  node &nstrobe, bvec<8> &data, node &nlf, node &nreset, node &nsel_prn,
  node nack, node nbusy, node paperout, node sel, node err
)
{
  // Pins (C36): GND=19, GND=20, GND=21, GND=22, GND=23, GND=24, GND=25, GND=26, GND=27, GND=28, GND=29, GND=30
  // 10, 11, 12, 13, 32,
  // 1, 2, 3, 4, 5, 6, 7, 8, 9, 14, 31, 36
  Module("centronics_parallel")
    .inputs(nack)(nbusy)(paperout)(sel)(err)
    .outputs(nstrobe)(data)(nlf)(nreset)(nsel_prn)
  ;
}

// Breadboard Terminal Strip
void Tstrip(bvec<54> tstrip) {
  // Note: we call them outputs to make life easier; they're really inout
  
  // Pins: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
  //       27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49
  //       50 51 52 53 54
  OUTPUT(tstrip);
  
  // We can't do the following because out module has no outputs; we should find
  // a way to print them all anyway, but for now we'll make the tstrip a single
  // giant output.
  // Module("terminal_strip").inputs(io);
}
#endif

int main() {
  const unsigned N_PORTS = 6;

  node clock, // Global shift clock
       latch_out, // Output register clock
       latch_in, // Input register clock
       nload_out; // Load output shift reg from output register (active low)
  
  bvec<N_PORTS> dir; // Output enables for Reg4096es, 1 for output, 0 for input
  vec<N_PORTS, bus<8> > io; // The i/o banks port0_0 through port5_7

  // The last 4094 in the chain controls the outputs of the rest.
  bus<8> final_outs;
  Cat(bvec<8-N_PORTS>(), dir) = final_outs;

  bvec<N_PORTS + 1> in_carry,  // Serial carry from one 4094 to the next.
                    out_carry; // Serial carry from one 597 to the next.

  node in_data, out_data; // Data ports from and to parallel port.
  
  in_carry[0] = in_data;
  out_carry[0] = Lit(0);
  out_data = out_carry[N_PORTS];

  // The output circuitry and port direction control.
  for (unsigned i = 0; i < N_PORTS; ++i) {
    node s2;
    Reg4094(io[i], in_carry[i+1], s2, in_carry[i], clock, latch_out, dir[i]);
  }

  {
    node s1, s2;
    Reg4094(final_outs, s1, s2, in_carry[N_PORTS], clock, latch_out, Lit(1));
  }

  // The input circuitry
  for (unsigned i = 0; i < N_PORTS; ++i) {
    Reg597(out_carry[i + 1],
	   out_carry[i], io[i], latch_out, clock, nload_out, Lit(1));
  }

  // The LPT interface.
  {
    bvec<8> lptdata;
    in_data = lptdata[0];
    Centronics(clock, lptdata, latch_out, latch_in, nload_out,
	       out_data, Lit(0), Lit(0), Lit(0), Lit(0));
  }

  // The terminal strip interface.
  {
    bvec<54> tstrip;

    // 6 ports can fit on this
    for (unsigned i = 0; i < 6; ++i)
      for (unsigned j = 0; j < 8; ++j)
	tstrip[8*i + j] = io[i][j];

    // Fill the rest of the strip with 3x VDD and 3x ground.
    for (unsigned i = 0; i < 3; ++i) {
      tstrip[8*6 + i] = Lit(0);
      tstrip[8*6 + 3 + i] = Lit(1);
    }

    Tstrip(tstrip);
  }
  
  optimize();
  
  ofstream nand("netlist.nand");
  print_netlist(nand);

  return 0;
}
