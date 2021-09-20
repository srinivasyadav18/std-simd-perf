for dir in $(ls -d */)
do
    echo $dir
    
    cd $dir
    
    echo $(pwd)
    if [ -d "plots" ]; then
        echo "plots for $dir already exists"
    else
        rm -rf build
        mkdir -p build
        cmake -S . -B ./build -GNinja -DCMAKE_BUILD_TYPE=Release > /dev/null
        cd build
        ninja
        cd ..
        build/exe $SIMD_END > /dev/null
        rm -rf build *.log
    fi
    cd ..
done
python3 generate_plots.py
python3 generate_plot.py
