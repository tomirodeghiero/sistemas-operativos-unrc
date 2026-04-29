/*
 * CounterExample.java
 *
 * Equivalente Java al ejercicio 6: dos threads incrementan un contador
 * compartido 100 000 veces cada uno. La region critica se protege con
 * synchronized sobre un objeto cerrojo dedicado, evitando el lost update.
 *
 * Uso:
 *   javac CounterExample.java
 *   java CounterExample
 */
public class CounterExample {
    private static int counter = 0;
    private static final Object lock = new Object();
    private static final int N = 100_000;

    public static void main(String[] args) throws InterruptedException {
        Thread t1 = new Thread(new Incrementer(1));
        Thread t2 = new Thread(new Incrementer(2));

        t1.start();
        t2.start();

        t1.join();
        t2.join();

        System.out.println("Final Counter Value: " + counter);
    }

    static class Incrementer implements Runnable {
        private final int tid;

        Incrementer(int tid) { this.tid = tid; }

        @Override
        public void run() {
            System.out.println("Thread " + tid + " running...");
            for (int i = 0; i < N; i++) {
                synchronized (lock) {
                    counter++;
                }
            }
        }
    }
}
