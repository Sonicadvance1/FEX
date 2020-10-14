import json
import os
import os.path
import sys

if (len(sys.argv) < 2):
    sys.exit()

bench_results = json.load(open(sys.argv[1]))

runner_style = sys.argv[2];

node = bench_results["machine_info"]["node"]
commit = bench_results["commit_info"]["id"]
branch = bench_results["commit_info"]["branch"]

test_name = bench_results["benchmarks"][0]["name"]
avg = bench_results["benchmarks"][0]["stats"]["mean"]

# Upload results
os.system("ssh loadci@loadci.fex-emu.org \"psql -d LoadCI -h localhost -U loadci_insert -c \\\"select public.insertresult('%s', '%s', '%s', '%s', %d);\\\"\""
    % (node, runner_style, test_name, commit, int(avg * 1000)))
