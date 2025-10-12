/*
gRPC Stubs - Minimal Implementation for Build Compatibility
This is a stub for demo purposes.
In a real project, use actual gRPC libraries.
*/

#ifndef GRPC_STUBS_H
#define GRPC_STUBS_H

namespace grpc {
    class Status {
    public:
        Status() = default;
        static const Status& OK;
        bool ok() const { return true; }
    };

    // Global OK status instance
    extern const Status _global_ok_status;
}

namespace google {
    namespace protobuf {
        namespace internal {
            class RepeatedPtrFieldBase {
            protected:
                void CloseGap(int start, int num) {}
            };
        }
    }
}

#endif // GRPC_STUBS_H