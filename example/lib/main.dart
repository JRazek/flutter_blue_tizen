import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

class BluetoothHelper {}

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  FlutterBlue flutterBlue = FlutterBlue.instance;
  // flutterBlue.setLogLevel(LogLevel.alert);
  List<ScanResult> scanResults = <ScanResult>[];
  var subscription = flutterBlue.scanResults.listen((results) async {
    // do something with scan results
    if (results.isNotEmpty) {
      // BluetoothDevice dev = results.last.device;
      // debugPrint('${dev.name} found! rssi: ${dev.name}');
      // debugPrint('connect done');
      scanResults.add(results.last);
    }
  });
  await flutterBlue.startScan(timeout: const Duration(seconds: 10));
  debugPrint("here bros");
  for (var sc in scanResults) {
    var dev = sc.device;
    debugPrint('${dev.name} found! rssi: ${sc.rssi}');
  }
  // Stop scanning
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: const Center(
          child: Text('Running on: '),
        ),
      ),
    );
  }
}
