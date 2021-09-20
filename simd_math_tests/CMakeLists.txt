set(subdirs 
    cos
    division
    exp
    log
    pow
    sin
    sin_cos
    tan
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)
