set(subdirs 
    compute_bound
    # for_each_n
    memory_bound
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)
