<h2 align="center">[RTAS19] Job-Class-Level Fixed Priority Scheduling of Weakly-Hard Real-Time Systems</h2>

### Directory layout
    .
    ├── analysis                # Schedulability analysis written in C
    ├── data                    # Sample data file (txt) for schedulability test
    ├── rpi_jcls                # JCLS implementation on Linux kernel v4.19
    ├── taskset_generation      # Taskset generation code in matlab
    └── README.md



### 1. Analysis
- Source codes for schedulability test under JCLS
- Build source codes
  ```
  make
  ```
- Run the sample data (sample data files are in `/data` folder)
  ```
  (Usage)>> ./check_sched filename.txt num_tasksets num_util_range filename_num.txt num_cpu
  (Example)>> ./check_sched data_40.txt 1000 11 data_40_num.txt 1
  ```

### 2. taskset_generation
- Taskset generation for schedulability test (written in matlab)
- Run `main_script.m` to generate a sample taskset

### 3. rpi_jcls
- Implementation of JCLS based on Linux kernel v4.19
- Tested on raspberry Pi 3 Model B
##### Sub folder layout
    rpi_jcls/
    ├── app                # Example for running JCLS
    ├── kernel             # Syscall definitions
    └── modules            # Loadable kernel modules

**NOTE**: Please reference our JCLS paper that was published in RTAS 2019.
```
@inproceedings{choi2019job,
  title={Job-class-level fixed priority scheduling of weakly-hard real-time systems},
  author={Choi, Hyunjong and Kim, Hyoseung and Zhu, Qi},
  booktitle={2019 IEEE Real-Time and Embedded Technology and Applications Symposium (RTAS)},
  pages={241--253},
  year={2019},
  organization={IEEE}
}

@article{choi2021toward,
  title={Toward Practical Weakly Hard Real-Time Systems: A Job-Class-Level Scheduling Approach},
  author={Choi, Hyunjong and Kim, Hyoseung and Zhu, Qi},
  journal={IEEE Internet of Things Journal},
  volume={8},
  number={8},
  pages={6692--6708},
  year={2021},
  publisher={IEEE}
}


```

  