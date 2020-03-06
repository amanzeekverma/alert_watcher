class=$1
shift
java -cp .:./slf4j-api-1.7.26.jar:./slf4j-simple-1.7.26.jar:./amqp-client-5.7.1.jar $class $@
