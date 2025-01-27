package contention.benchmark.json.serializers;

import com.google.gson.*;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;

import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.Map;

public class DataMapBuilderSerializer extends BuilderSerializer<DataMapBuilder> {
    private final static Map<Integer, DataMapBuilder> builders = new HashMap<>();

    @Override
    protected String getClassPath() {
        return "contention.benchmark.workload.data.map.builders.";
    }

    @Override
    public DataMapBuilder deserialize(JsonElement jsonElement, Type type, JsonDeserializationContext context) throws JsonParseException {
        JsonObject object = jsonElement.getAsJsonObject();
        int id = object.get("id").getAsInt();

        DataMapBuilder builder;

        if ((builder = builders.get(id)) == null) {
            builder = super.deserialize(jsonElement, type, context);
            builders.put(id, builder);
        }

        return builder;
    }

    public static Map<Integer, DataMapBuilder> getBuilders() {
        return builders;
    }
}
