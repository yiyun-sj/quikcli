#ifndef QC_CONSTANTS_H_
#define QC_CONSTANTS_H_

namespace quikcli {

struct DefaultParams {
  static constexpr char name[] = "cli";
  static constexpr char version[] = "0.1.0";
};

struct ArgNames {
  static constexpr char version[] = "--version";
  static constexpr char help[] = "--help";
};

}

#endif // QC_CONSTANTS_H_
