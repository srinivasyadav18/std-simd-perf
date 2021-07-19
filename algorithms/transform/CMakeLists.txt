set(subdirs 
    compute_bound
    memory_bound
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)
