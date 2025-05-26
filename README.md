
# Usage

````bash
sudo apt install clang verilator yosys
````

## Testing

````
make obj_dir/Vmatmul
obj_dir/Vmatmul
````

## Synthesis

Download FreePDK45 from https://eda.ncsu.edu/downloads/ (you need to enter your email address and accept a license). Extract in the `pdk` directory, so you have `pdk/FreePDK45`.

Run synthesis with:

````bash
make synth
````