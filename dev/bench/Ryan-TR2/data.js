window.BENCHMARK_DATA = {
  "lastUpdate": 1602715904488,
  "repoUrl": "https://github.com/Sonicadvance1/FEX",
  "entries": {
    "Benchmark": [
      {
        "commit": {
          "author": {
            "email": "Sonicadvance1@gmail.com",
            "name": "Ryan Houdek",
            "username": "Sonicadvance1"
          },
          "committer": {
            "email": "Sonicadvance1@gmail.com",
            "name": "Ryan Houdek",
            "username": "Sonicadvance1"
          },
          "distinct": true,
          "id": "bc21c74bfa1c018694648d202fe2ab0382e9f8ff",
          "message": "Split up bench results by node",
          "timestamp": "2020-10-14T15:38:49-07:00",
          "tree_id": "6edcd5f8c21acfa555b6e7a1e4871c3cbb560c35",
          "url": "https://github.com/Sonicadvance1/FEX/commit/bc21c74bfa1c018694648d202fe2ab0382e9f8ff"
        },
        "date": 1602715251060,
        "tool": "pytest",
        "benches": [
          {
            "name": "FEX/Scripts/Execute_FEX.py::test_povray_bench",
            "value": 0.0793195332707,
            "unit": "iter/sec",
            "range": "stddev: 0.10909322235437714",
            "extra": "mean: 12.607235049999872 sec\nrounds: 5"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "Sonicadvance1@gmail.com",
            "name": "Ryan Houdek",
            "username": "Sonicadvance1"
          },
          "committer": {
            "email": "Sonicadvance1@gmail.com",
            "name": "Ryan Houdek",
            "username": "Sonicadvance1"
          },
          "distinct": true,
          "id": "7f504f05d1ca8b4c4c8a214e2d22714dd5032da1",
          "message": "More changes",
          "timestamp": "2020-10-14T15:43:28-07:00",
          "tree_id": "d362b70d84585ccf1f8e06942083284e16d67b1a",
          "url": "https://github.com/Sonicadvance1/FEX/commit/7f504f05d1ca8b4c4c8a214e2d22714dd5032da1"
        },
        "date": 1602715528906,
        "tool": "pytest",
        "benches": [
          {
            "name": "FEX/Scripts/Execute_FEX.py::test_native_povray_bench",
            "value": 0.7989485128099806,
            "unit": "iter/sec",
            "range": "stddev: 0.03242322338774505",
            "extra": "mean: 1.2516451110008346 sec\nrounds: 5"
          },
          {
            "name": "FEX/Scripts/Execute_FEX.py::test_povray_bench",
            "value": 0.07970667260788612,
            "unit": "iter/sec",
            "range": "stddev: 0.17534239873040666",
            "extra": "mean: 12.546001072199578 sec\nrounds: 5"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "Sonicadvance1@gmail.com",
            "name": "Ryan Houdek",
            "username": "Sonicadvance1"
          },
          "committer": {
            "email": "Sonicadvance1@gmail.com",
            "name": "Ryan Houdek",
            "username": "Sonicadvance1"
          },
          "distinct": true,
          "id": "c3468cedca8e57b3123f7004cb591567f1237a48",
          "message": "Reduce pytest rounds",
          "timestamp": "2020-10-14T15:50:25-07:00",
          "tree_id": "eeb3cccdd4e6867779fd88e5f95584cafd9c1799",
          "url": "https://github.com/Sonicadvance1/FEX/commit/c3468cedca8e57b3123f7004cb591567f1237a48"
        },
        "date": 1602715903941,
        "tool": "pytest",
        "benches": [
          {
            "name": "FEX/Scripts/Execute_FEX.py::test_native_povray_bench",
            "value": 0.8133394941780903,
            "unit": "iter/sec",
            "range": "stddev: 0.00006474340389524158",
            "extra": "mean: 1.2294988835019467 sec\nrounds: 2"
          },
          {
            "name": "FEX/Scripts/Execute_FEX.py::test_povray_bench",
            "value": 0.07888725451869318,
            "unit": "iter/sec",
            "range": "stddev: 0.14559327917783912",
            "extra": "mean: 12.676318958001502 sec\nrounds: 2"
          }
        ]
      }
    ]
  }
}