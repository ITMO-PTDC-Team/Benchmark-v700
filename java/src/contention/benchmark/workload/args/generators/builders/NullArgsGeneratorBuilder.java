package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.NullArgsGenerator;

public class NullArgsGeneratorBuilder implements ArgsGeneratorBuilder {

    public NullArgsGeneratorBuilder() {
    }

    @Override
    public ArgsGeneratorBuilder init(int range) {
        return this;
    }

    @Override
    public ArgsGenerator build() {
        return new NullArgsGenerator();
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        StringBuilder sb = new StringBuilder();
        String indentStr = "    ".repeat(indents);
        sb.append(indentStr).append("Type: NULL\n");
        return sb;
    }
}
