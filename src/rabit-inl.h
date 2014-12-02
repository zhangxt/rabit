/*!
 * \file rabit-inl.h
 * \brief implementation of inline template function for rabit interface
 *   
 * \author Tianqi Chen
 */
#ifndef RABIT_RABIT_INL_H
#define RABIT_RABIT_INL_H

namespace rabit {
namespace engine {
namespace mpi {
// template function to translate type to enum indicator
template<typename DType>
inline DataType GetType(void);
template<>
inline DataType GetType<int>(void) {
  return kInt;
}
template<>
inline DataType GetType<unsigned>(void) {
  return kUInt;
}
template<>
inline DataType GetType<float>(void) {
  return kFloat;
}
template<>
inline DataType GetType<double>(void) {
  return kDouble;
}
}  // namespace mpi
}  // namespace engine

namespace op {
struct Max {
  const static engine::mpi::OpType kType = engine::mpi::kMax;
  template<typename DType>
  inline static void Reduce(DType &dst, const DType &src) {
    if (dst < src) dst = src;
  }
};
struct Min {
  const static engine::mpi::OpType kType = engine::mpi::kMin;
  template<typename DType>
  inline static void Reduce(DType &dst, const DType &src) {
    if (dst > src) dst = src;
  }
};
struct Sum {
  const static engine::mpi::OpType kType = engine::mpi::kSum;
  template<typename DType>
  inline static void Reduce(DType &dst, const DType &src) {
    dst += src;
  }
};
struct BitOR {
  const static engine::mpi::OpType kType = engine::mpi::kBitwiseOR;
  template<typename DType>
  inline static void Reduce(DType &dst, const DType &src) {
    dst |= src;
  }
};
template<typename OP, typename DType>
inline void Reducer(const void *src_, void *dst_, int len, const MPI::Datatype &dtype) {
  const DType *src = (const DType*)src_;
  DType *dst = (DType*)dst_;  
  for (int i = 0; i < len; ++i) {
    OP::Reduce(dst[i], src[i]);
  }
}
} // namespace op

// intialize the rabit engine
inline void Init(int argc, char *argv[]) {
  engine::Init(argc, argv);
}
// finalize the rabit engine
inline void Finalize(void) {
  engine::Finalize();
}
// get the rank of current process
inline int GetRank(void) {
  return engine::GetEngine()->GetRank();
}
// the the size of the world
inline int GetWorldSize(void) {
  return engine::GetEngine()->GetWorldSize();
}
// get the name of current processor
inline std::string GetProcessorName(void) {
  return engine::GetEngine()->GetHost();
}
// broadcast an std::string to all others from root
inline void Bcast(std::string *sendrecv_data, int root) {
  engine::IEngine *e = engine::GetEngine();
  unsigned len = static_cast<unsigned>(sendrecv_data->length());
  e->Broadcast(&len, sizeof(len), root);
  sendrecv_data->resize(len);
  if (len != 0) {
    e->Broadcast(&(*sendrecv_data)[0], len, root);  
  }
}
// perform inplace AllReduce
template<typename OP, typename DType>
inline void AllReduce(DType *sendrecvbuf, size_t count) {
  engine::AllReduce_(sendrecvbuf, sizeof(DType), count, op::Reducer<OP,DType>,
                     engine::mpi::GetType<DType>(), OP::kType);
}
// load latest check point
inline int LoadCheckPoint(utils::ISerializable *p_model) {
  return engine::GetEngine()->LoadCheckPoint(p_model);
}
// checkpoint the model, meaning we finished a stage of execution
inline void CheckPoint(const utils::ISerializable &model) {
  engine::GetEngine()->CheckPoint(model);
}
// return the version number of currently stored model
inline int VersionNumber(void) {
  return engine::GetEngine()->VersionNumber();
}
}  // namespace rabit
#endif