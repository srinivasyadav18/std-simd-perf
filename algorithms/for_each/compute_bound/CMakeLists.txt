set(subdirs 
    default_chunk
    default_chunk_eve_align
    default_chunk_hpx_vector
    # static_chunk
    # static_chunk_eve_align
    # static_chunk_hpx_vector
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)
