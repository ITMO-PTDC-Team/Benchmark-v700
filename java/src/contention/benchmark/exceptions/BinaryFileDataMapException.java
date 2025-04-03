package contention.benchmark.exceptions;

public class BinaryFileDataMapException extends RuntimeException {
    public BinaryFileDataMapException(String message) {
        super(message);
    }

    public BinaryFileDataMapException(Throwable cause) {
        super(cause);
    }

    public BinaryFileDataMapException(String message, Throwable cause) {
        super(message, cause);
    }

}
