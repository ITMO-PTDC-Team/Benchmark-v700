package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.GeneralizedArgsGenerator;

import java.util.*;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class GeneralizedArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    private static final Set<String> OPER_TYPES = Set.of("get", "insert", "remove", "rangeQuery");

    private static class BuilderOperations {
        ArgsGeneratorBuilder builder;
        List<String> operations;

        BuilderOperations(ArgsGeneratorBuilder builder, List<String> operations) {
            this.builder = builder;
            this.operations = operations;
        }
    }

    private List<BuilderOperations> argsGeneratorBuilders = new ArrayList<>();
    private Set<String> undecOperTypes = new HashSet<>(OPER_TYPES);

    public GeneralizedArgsGeneratorBuilder addArgsGeneratorBuilder(List<String> opers,
                                                                   ArgsGeneratorBuilder argsGenBuilder) {
        for (String operType : opers) {
            if (!OPER_TYPES.contains(operType)) {
                throw new IllegalArgumentException("Unsupported operation type: " + operType);
            }
            if (!undecOperTypes.contains(operType)) {
                throw new IllegalArgumentException("Multiple declaration of operation type: " + operType);
            }
            undecOperTypes.remove(operType);
        }

        argsGeneratorBuilders.add(new BuilderOperations(argsGenBuilder, opers));
        return this;
    }

    @Override
    public ArgsGeneratorBuilder init(int range) {
        if (!undecOperTypes.isEmpty()) {
            addArgsGeneratorBuilder(
                    new ArrayList<>(undecOperTypes),
                    new NullArgsGeneratorBuilder()
            );
        }

        for (BuilderOperations currentBuilder : argsGeneratorBuilders) {
            currentBuilder.builder.init(range);
        }
        return this;
    }

    @Override
    public ArgsGenerator build() {
        Map<String, ArgsGenerator> built = new HashMap<>();
        for (BuilderOperations currentBuilder : argsGeneratorBuilders) {
            ArgsGenerator generator = currentBuilder.builder.build();
            for (String operType : currentBuilder.operations) {
                built.put(operType, generator);
            }
        }

        return new GeneralizedArgsGenerator(
                built.get("get"),
                built.get("insert"),
                built.get("remove"),
                built.get("rangeQuery")
        );
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        StringBuilder sb = new StringBuilder()
                .append(indentedTitleWithData("Type", "GENERALIZED", indents));

        for (BuilderOperations currentBuilder : argsGeneratorBuilders) {
            String opers = String.join(", ", currentBuilder.operations);
            sb.append(indentedTitle("Args Generators (" + opers + ")", indents));
            sb.append(currentBuilder.builder.toStringBuilder(indents + 1));
        }

        return sb;
    }

}