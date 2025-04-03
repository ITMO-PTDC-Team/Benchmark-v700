package contention.benchmark.exceptions;

public class FileBasedDataMapException extends RuntimeException {
    public FileBasedDataMapException(String message) {
        super(message);
    }

    public FileBasedDataMapException(Throwable cause) {
        super(cause);
    }

    public FileBasedDataMapException(String message, Throwable cause) {
        super(message, cause);
    }

}
