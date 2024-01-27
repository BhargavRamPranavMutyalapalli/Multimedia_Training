import subprocess

def run_gstreamer_pipeline(pipeline):
    try:
        subprocess.run(pipeline, shell=True, check=True)
        return True  # Return True if the pipeline runs successfully
    except subprocess.CalledProcessError as e:
        print(f"Error running GStreamer pipeline: {e}")
        return False  # Return False if the pipeline fails

def write_result_to_file(result):
    with open("test_results.txt", "a") as file:
        file.write(result + "\n")

def test_cases():
    # Test Case 1: Default pipeline
    pipeline_1 = "gst-launch-1.0 videotestsrc ! video/x-raw,width=1920,height=1080,format=NV12 ! autovideosink"
    print("MY_PRINT",pipeline_1)
    #result_1 = "pass" if run_gstreamer_pipeline(pipeline_1) else "fail"
    #write_result_to_file(f"Test Case 1: {result_1}")
    check = run_gstreamer_pipeline(pipeline_1)
    print("check",check)
    if check:
        result_1 = "pass"
        write_result_to_file(f"Test Case 1: {result_1}")
    else:
        result_1 = "fail"
        write_result_to_file(f"Test Case 1: {result_1}")


    # Test Case 2: Change resolution
    pipeline_2 = "gst-launch-1.0 videotestsrc ! video/x-raw,width=1280,height=720,format=NV12 ! autovideosink"
    result_2 = "pass" if run_gstreamer_pipeline(pipeline_2) else "fail"
    write_result_to_file(f"Test Case 2: {result_2}")

    # Test Case 3: Add a filter or element
    pipeline_3 = "gst-launch-1.0 videotestsrc ! video/x-raw,width=1920,height=1080,format=NV12 ! autovideosink textoverlay text=\"Test Overlay\""
    result_3 = "pass" if run_gstreamer_pipeline(pipeline_3) else "fail"
    write_result_to_file(f"Test Case 3: {result_3}")

    # Add more test cases as needed...

if __name__ == "__main__":
    test_cases()
