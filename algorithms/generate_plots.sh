for dir in $(ls -d */)
do
    echo $dir
    
    cd $dir
    
    echo $(pwd)
    mkdir -p build
    cmake -S . -B ./build -GNinja -DCMAKE_BUILD_TYPE=Release > /dev/null
    cd build
    ninja
    cd ..
    build/exe --end=26 --iterations=10
    rm -rf build *.log
    cd ..
done
python3 generate_plots.py
python3 generate_plot.py