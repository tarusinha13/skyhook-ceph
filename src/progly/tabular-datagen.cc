#include <iostream>
#include <random>
#include <boost/program_options.hpp>
#include "include/rados/librados.hpp"

namespace po = boost::program_options;

#define checkret(r,v) do { \
  if (r != v) { \
    fprintf(stderr, "error %d/%s\n", r, strerror(-r)); \
    assert(0); \
    exit(1); \
  } } while (0)

int main(int argc, char **argv)
{
  uint64_t range_size;
  uint64_t seed;
  unsigned num_rows;
  unsigned rows_per_obj;
  std::string pool;

  po::options_description gen_opts("General options");
  gen_opts.add_options()
    ("help,h", "show help message")
    ("range-size", po::value<uint64_t>(&range_size)->required(), "data range")
    ("seed", po::value<uint64_t>(&seed)->default_value(1234), "rand seed")
    ("num-rows", po::value<unsigned>(&num_rows)->required(), "number of rows")
    ("rows-per-obj", po::value<unsigned>(&rows_per_obj)->required(), "rows per object")
		("pool,p", po::value<std::string>(&pool)->required(), "pool")
  ;

  po::options_description all_opts("Allowed options");
  all_opts.add(gen_opts);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, all_opts), vm);

  if (vm.count("help")) {
    std::cout << all_opts << std::endl;
    return 1;
  }

  po::notify(vm);

  assert(range_size > 0);
  assert(num_rows > 0);
  assert(rows_per_obj > 0);
  assert(num_rows % rows_per_obj == 0);

  // random number engine
  std::mt19937_64 mte(seed);
  assert(mte.min() == 0);
  uint64_t range_max = range_size - 1;
  assert(range_max < mte.max());

  // distribution
  std::uniform_int_distribution<uint64_t> dist(0, range_max);
  assert(dist.min() == 0);
  assert(dist.max() == range_max);

  // connect to rados
  librados::Rados cluster;
  cluster.init(NULL);
  cluster.conf_read_file(NULL);
  int ret = cluster.connect();
  checkret(ret, 0);

  // open pool i/o context
  librados::IoCtx ioctx;
  ret = cluster.ioctx_create(pool.c_str(), ioctx);
  checkret(ret, 0);

  // build each object
  const unsigned num_objs = num_rows / rows_per_obj;
  for (unsigned o = 0; o < num_objs; o++) {

    // fill a blob with random numbers
    ceph::bufferlist bl;
    for (unsigned r = 0; r < rows_per_obj; r++) {
      uint64_t val = dist(mte);
      bl.append((char*)&val, sizeof(val));
    }

    std::stringstream ss;
    ss << "obj." << o;
    const std::string oid = ss.str();

    std::cout << "writing object: " << oid
      << " " << (o+1) << "/" << num_objs
      << "\r" << std::flush;

    int ret = ioctx.remove(ss.str());
    if (ret != -ENOENT)
      checkret(ret, 0);

    // write object
    ceph::bufferlist outbl;
    ret = ioctx.exec(ss.str(), "tabular", "add", bl, outbl);
    checkret(ret, 0);
  }
  std::cout << std::endl;

  ioctx.close();
  cluster.shutdown();

  return 0;
}
